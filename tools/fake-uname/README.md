# fake-uname.so

compile with
```bash
make
```

use like
```bash
$ FAKE_UNAME_RELEASE=happily-overriding-release FAKE_UNAME_VERSION=happily-overriding-version-too LD_PRELOAD=/path/to/your/fake-uname.so python -c "import os; print(os.uname())"
posix.uname_result(sysname='Linux', nodename='musashi2.rokugan.lan', release='happily-overriding-release', version='happily-overriding-version-too', machine='x86_64')

$ FAKE_UNAME_RELEASE=happily-overriding-release FAKE_UNAME_VERSION=happily-overriding-version-too LD_PRELOAD=/path/to/your/fake-uname.so uname -a
Linux musashi2.rokugan.lan happily-overriding-release happily-overriding-version-too x86_64 x86_64 x86_64 GNU/Linux
```

please note `fake-uname.so` will do nothing unless told so:
```bash
$ uname -a
Linux musashi2.rokugan.lan 5.11.17-200.fc33.x86_64 #1 SMP Wed Apr 28 17:34:39 UTC 2021 x86_64 x86_64 x86_64 GNU/Linux
$ LD_PRELOAD=./fake-uname.so uname -a
Linux musashi2.rokugan.lan 5.11.17-200.fc33.x86_64 #1 SMP Wed Apr 28 17:34:39 UTC 2021 x86_64 x86_64 x86_64 GNU/Linux
```
