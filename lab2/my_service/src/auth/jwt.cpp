#include "jwt.hpp"

#include <chrono>
#include <cstdint>
#include <string>
#include <string_view>

#include <openssl/evp.h>
#include <openssl/hmac.h>

#include <userver/formats/json/serialize.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/formats/json/value_builder.hpp>

namespace myservice::auth {

namespace {

std::int64_t NowUnix() {
  const auto now = std::chrono::system_clock::now();
  return std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
}

std::string Base64UrlEncode(const unsigned char* data, std::size_t len) {
  const std::size_t out_len = 4 * ((len + 2) / 3);
  std::string out(out_len, '\0');
  const int real = EVP_EncodeBlock(reinterpret_cast<unsigned char*>(out.data()), data,
                                  static_cast<int>(len));
  out.resize(real);

  for (auto& c : out) {
    if (c == '+') c = '-';
    else if (c == '/') c = '_';
  }
  while (!out.empty() && out.back() == '=') out.pop_back();
  return out;
}

std::string Base64UrlEncode(std::string_view s) {
  return Base64UrlEncode(reinterpret_cast<const unsigned char*>(s.data()), s.size());
}

std::string Base64UrlDecode(std::string_view in) {
  std::string b64(in);
  for (auto& c : b64) {
    if (c == '-') c = '+';
    else if (c == '_') c = '/';
  }
  while (b64.size() % 4 != 0) b64.push_back('=');

  std::string out((b64.size() / 4) * 3, '\0');
  const int real = EVP_DecodeBlock(reinterpret_cast<unsigned char*>(out.data()),
                                  reinterpret_cast<const unsigned char*>(b64.data()),
                                  static_cast<int>(b64.size()));
  if (real < 0) return {};

  out.resize(real);

  while (!out.empty() && out.back() == '\0') out.pop_back();
  return out;
}

std::string HmacSha256Base64Url(std::string_view data, std::string_view secret) {
  unsigned int len = 0;
  unsigned char md[EVP_MAX_MD_SIZE];

  HMAC(EVP_sha256(),
       reinterpret_cast<const unsigned char*>(secret.data()),
       static_cast<int>(secret.size()),
       reinterpret_cast<const unsigned char*>(data.data()),
       data.size(),
       md, &len);

  return Base64UrlEncode(md, len);
}

bool ConstantTimeEqual(std::string_view a, std::string_view b) {
  if (a.size() != b.size()) return false;
  unsigned char diff = 0;
  for (std::size_t i = 0; i < a.size(); ++i) diff |= (a[i] ^ b[i]);
  return diff == 0;
}

}  // namespace

std::string CreateTokenHS256(const JwtClaims& claims, std::string_view secret) {
  userver::formats::json::ValueBuilder header;
  header["alg"] = "HS256";
  header["typ"] = "JWT";

  userver::formats::json::ValueBuilder payload;
  payload["sub"] = claims.user_id;
  payload["email"] = claims.email;
  payload["role"] = claims.role;
  payload["exp"] = claims.exp;

  const auto header_json = userver::formats::json::ToString(header.ExtractValue());
  const auto payload_json = userver::formats::json::ToString(payload.ExtractValue());

  const auto header_b64 = Base64UrlEncode(header_json);
  const auto payload_b64 = Base64UrlEncode(payload_json);

  const std::string signing_input = header_b64 + "." + payload_b64;
  const auto signature_b64 = HmacSha256Base64Url(signing_input, secret);

  return signing_input + "." + signature_b64;
}

bool VerifyTokenHS256(std::string_view token, std::string_view secret, JwtClaims& out) {
  const auto p1 = token.find('.');
  if (p1 == std::string_view::npos) return false;
  const auto p2 = token.find('.', p1 + 1);
  if (p2 == std::string_view::npos) return false;

  const auto header_b64 = token.substr(0, p1);
  const auto payload_b64 = token.substr(p1 + 1, p2 - (p1 + 1));
  const auto sig_b64 = token.substr(p2 + 1);

  const std::string signing_input = std::string(header_b64) + "." + std::string(payload_b64);
  const auto expected_sig = HmacSha256Base64Url(signing_input, secret);

  if (!ConstantTimeEqual(expected_sig, sig_b64)) return false;

  const auto payload_json = Base64UrlDecode(payload_b64);
  if (payload_json.empty()) return false;

  userver::formats::json::Value payload;
  try {
    payload = userver::formats::json::FromString(payload_json);
  } catch (...) {
    return false;
  }

  try {
    const auto exp = payload["exp"].As<std::int64_t>();
    if (exp < NowUnix()) return false;

    out.user_id = payload["sub"].As<std::int64_t>();
    out.email = payload["email"].As<std::string>();
    out.role = payload["role"].As<std::string>();
    out.exp = exp;
    return true;
  } catch (...) {
    return false;
  }
}

}  // namespace myservice::auth