# Мое руководство для старта работы с userver
Я работаю на mac os
1) Не хочу работать в дев контейнерах, поэтому:
```brew install lima```
```limactl start default```
```lima```
Результат: запущена линукс машина в терминале 
2) Подготовка окружения под c++ userver (собираем из исходников)

``` sudo apt install -y g++-12 cmake make git``` 

```sudo apt install -y g++-12 cmake make git     libboost1.74-all-dev libssl-dev libfmt-dev     libpq-dev postgresql-server-dev-all     lib yaml-cpp-dev libc-ares-dev```

```sudo apt install -y     libssl-dev     libfmt-dev     libpq-dev     postgresql-server-dev-all     libyaml-cpp-dev     libc-ares-dev```

```sudo apt install -y     libcurl4-openssl-dev     zlib1g-dev     libbz2-dev     liblzma-dev```

```cd ~```

```git clone https://github.com/userver-framework/userver.git```

```cd userver/```

```mkdir build && cd build```

```sudo apt update && sudo apt install libjemalloc-dev```

```sudo apt install -y     libjemalloc-dev     libcurl4-openssl-dev     libc-ares-dev     libcrypto++-dev     libev-dev     libevent-dev     libhiredis-dev     libmariadb-dev     libmongoc-dev     libbson-dev```

```sudo apt install -y     libjemalloc-dev     libcurl4-openssl-dev     libc-ares-dev     libcrypto++-dev     libev-dev     libevent-dev     libhiredis-dev     libmariadb-dev     libmongoc-dev     libbson-dev     librabbitmq-dev     librdkafka-dev     libsqlite3-dev```

```sudo apt install -y clang-format```

```sudo apt install -y libgtest-dev libgmock-dev```

```sudo apt install -y libbenchmark-dev```