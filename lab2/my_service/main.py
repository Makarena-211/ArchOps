import requests

new_post = {
    "patientId": 1,
    "doctorId": 10,
    "diagnosis": "ARVI",
    "notes": "rest"
}

url = "http://localhost:46317/api/records"

try:
    r = requests.post(url, json=new_post, timeout=5)
    print("status:", r.status_code)
    print("headers:", r.headers.get("content-type"))
    print("text:", r.text)

    try:
        print("json:", r.json())
    except Exception as e:
        print("json parse error:", e)

except requests.exceptions.RequestException as e:
    print("request failed:", type(e).__name__, e)