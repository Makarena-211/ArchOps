#include <userver/utils/daemon_run.hpp>

#include "components/component_list.hpp"

int main(int argc, char* argv[]) {
  return userver::utils::DaemonMain(argc, argv, myservice::components::MakeComponentList());
}