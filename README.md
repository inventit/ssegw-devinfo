ServiceSync Embedded Device Info App
===

このアプリケーションは、ServiceSync Embedded Gatewayで動作する、デバイス情報を取得するアプリケーションです。

## ディレクトリ構成

```
${APP_ROOT}
├── Makefile
├── README.md
├── certs .................... (1)
├── common.gypi
├── configure
├── include
│   └── servicesync .......... (2)
├── moatapp.gyp
├── package
│   └── package.json
├── src ...................... (3)
├── test
└── tools
```

## ビルド手順

### 事前準備

* プラットフォーム証明書`moat.pem`を、`certs`ディレクトリに予め配置しておく必要があります。
  * プラットフォーム証明書は、動作環境(DMS等)に合わせて異なりますので、都度、環境に合わせて配置しなおしてください。
* [iidn-cli](https://github.com/inventit/iidn-cli)をインストールする必要があります。
  * インストール後、`iidn`コマンドに実行パスを通しておいてください。
* アプリケーションをビルドするためには、API認証情報(appId/clientId/password)が必要です。予め、ServiceSyncのアカウントを取得してください。

### `token.bin`の生成

```
$ cd ${APP_ROOT}/package
$ $ iidn tokengen file
[IIDN] ** Using node... **
[IIDN] Enter your appId:
704dc8aa-xxxx-xxxx-xxxx-xxxxxxxxx
[IIDN] Enter your clientId:
admin@xxxxxxxxxxxxxxxxxxxxxxxxxxx
[IIDN] Enter your password:
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

 ... (snip) ...

[IIDN] Done
```

`iidn tokengen`を実行すると、実行したディレクトリに`1427021905250-token.bin`のようなファイル名で、トークンが生成されます。
このファイル名を、`token.bin`に変更してください。

```
.
├── Makefile
├── README.md
├── certs
│   └── moat.pem ....... (*)
├── common.gypi
├── configure
├── include
├── moatapp.gyp
├── package
│   ├── package.json
│   └── token ...... (*)
├── src
├── test
└── tools
```

### ビルド

```
$ git clone https://${USER}@bitbucket.org/inventit/ssegw-devinfo.git
$ cd ssegw-devinfo
$ git checkout <branch name> # if necessary
$ git submodule init
$ git submodule update
$ ./configure ...... (*)
$ make
```

ターゲットに合わせてクロスコンパイラの設定と configure のオプションの変更を行ってください。

* Debian on PC (x86_64 / i686)

```
$ configure
```

* Armadillo-IoT

```
$ export CROSS=arm-linux-gnueabi-
$ export CC=${CROSS}gcc
$ export CXX=${CROSS}g++
$ export AR=${CROSS}ar
$ export LD=${CROSS}ld
$ export RANLIB=${CROSS}ranlib
$ export STRIP=${CROSS}strip
$ ./configure --dest-cpu=arm --product=Armadillo-IoT 
```

### パッケージ作成

Gatewayにインストール可能なパッケージを作成します。
以下のコマンドを実行すると、`devinfo_<version>_<arch>.zip`というファイル名のzipファイルが作成されます。

```
$ make package
```

## 変更履歴

Changes in `2.0.0` : July 17, 2015

* ServiceSync v2.1 support
* Armadillo-IoT support

Changes in `0.0.1` : 2015/04/01

* Initial Release.
