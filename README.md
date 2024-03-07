# Derivative Monitoring System(파생 데이터 모니터링)

## 목차
- [Derivative Monitoring System(파생 데이터 모니터링)](#derivative-monitoring-system파생-데이터-모니터링)
  - [목차](#목차)
  - [1. 개요](#1-개요)
  - [2. 실행파일](#2-실행파일)
    - [2.1. monserver](#21-monserver)
      - [2.1.1. 개요](#211-개요)
      - [2.1.2. 환경파일](#212-환경파일)
    - [2.2. moninit](#22-moninit)
      - [2.2.1. 개요](#221-개요)
      - [2.2.2. 사용법](#222-사용법)
    - [2.3. monrecv](#23-monrecv)
      - [2.3.1. 개요](#231-개요)
      - [2.3.2. 환경파일](#232-환경파일)
      - [2.3.3. 사용법](#233-사용법)
    - [2.4. monfep](#24-monfep)
      - [2.4.1. 개요](#241-개요)
      - [2.4.2. 환경파일](#242-환경파일)
      - [2.4.3. 사용법](#243-사용법)
    - [2.5. monquot](#25-monquot)
      - [2.5.1. 개요](#251-개요)
      - [2.5.2. 사용법](#252-사용법)
    - [2.6. monipcx](#26-monipcx)
      - [2.6.1. 개요](#261-개요)
      - [2.6.2. 사용법](#262-사용법)
      - [2.6.3. 종속파일](#263-종속파일)
  - [3. 환경파일](#3-환경파일)
    - [3.1. MONMAIN.json](#31-monmainjson)
      - [3.1.1. 개요](#311-개요)
      - [3.1.2. 양식](#312-양식)
      - [3.1.3. 필드 설명](#313-필드-설명)
    - [3.2. \[거래소명\].json](#32-거래소명json)
    - [3.3. 필드 설명](#33-필드-설명)


## 1. 개요
해당 프로그램은 `파생 데이터 모니터링` 을 위해 제작되었습니다.

## 2. 실행파일
### 2.1. monserver
#### 2.1.1. 개요
`monserver` 은 개별 프로세스 관리용 데몬 서버이다. `/mdfsvc/Monitor/shl/runserver.sh` 스크립트로 실행할 수 있다.

#### 2.1.2. 환경파일
`/mdfsvc/Monitor/etc/MONMAIN.json` 을 참고하여 거래소별로 프로세스를 `ON/OFF` 할 수 있다.

### 2.2. moninit
#### 2.2.1. 개요
`moninit` 은 [`monserver`](#21-monserver) 로 시그널을 발송하여 데몬 서버에서 관리하는 프로세스를 `ON/OFF` 할 수 있다.

#### 2.2.2. 사용법
```
Usage: moninit [options]
Options:
  -k    Kill all processes
  -s    Run all processes
  --help  Display this help message
```

### 2.3. monrecv
#### 2.3.1. 개요
`monrecv` 는 환경파일에 따라 `Multicast IP/PORT`로 조인하여 데이터를 수신하는 프로세스이다. 데이터 수신 후 `monfep`으로 데이터를 전달한다.

#### 2.3.2. 환경파일
`/mdfsvc/Monitor/etc/[거래소명].json` 을 참고한다.

#### 2.3.3. 사용법
```
usage : monrecv [exchange_name]
```

### 2.4. monfep
#### 2.4.1. 개요
`monfep` 은 환경파일에 따라 데이터를 가공하는 프로세스이다. `monrecv` 로부터 데이터를 수신한다.

#### 2.4.2. 환경파일
`/mdfsvc/Monitor/etc/[거래소명].json` 을 참고한다.

#### 2.4.3. 사용법
```
usage : monfep [exchange_name]
```

### 2.5. monquot
#### 2.5.1. 개요
`monquot` 은 실시간 호가, 체결 업데이트 사항을 터미널로 모니터링할 수 있는 툴이다.

#### 2.5.2. 사용법
```
usage : monquot
```

### 2.6. monipcx
#### 2.6.1. 개요
`monipcx` 는 `monfep` 기동 시 세팅되는 공유메모리에 대하여, 메모리 내 저장된 모든 메모리를 삭제하는 명령어이다.

#### 2.6.2. 사용법
```
usage : monipcx [exchange_name]
```

#### 2.6.3. 종속파일
  `monlog.c` `monclass.c` `moncheck.c` `monsend.c` `monmap.c` `oldmap.c` `extmap.c`

## 3. 환경파일
### 3.1. MONMAIN.json
#### 3.1.1. 개요
`monserver` 기동 시 참조하는 환경파일이다.

#### 3.1.2. 양식
```
[                                                                                                                                                                                                                                           
    {
        "exchange": "",
        "running":
    },
    {
        "exchange": "",
        "running":
    }
]
```
#### 3.1.3. 필드 설명
|필드명|FORMAT|설명|비고|
|:--:|:--:|:--:|:--:|
|exchange|STRING|거래소명|
|running|NUM|실행 여부|1 또는 0


### 3.2. [거래소명].json
```
{
    "settings": {
        "name": "",
        "type": "",
        "description": "",
        "timezone": "",
        "room": ,
        "logLevel": ""
    },
    "raw_data": {
        "max_date" : ,
        "depth_log": ,
        "cross_check": 
    },
    "ports": [
        {
            "running": "",
            "alert": "",
            "name": "",
            "host": "",
            "type": "",
            "format": "",
            "ipad": "",
            "port": ,
            "nic": "",
            "times": [
                {
                    "wday": {
                        "start": "",
                        "end": ""
                    },
                    "window": {
                        "start": "",
                        "end": ""
                    }
                },
                {
                    "wday": {
                        "start": "",
                        "end": ""
                    },
                    "window": {
                        "start": "",
                        "end": ""
                    }
                }
            ]
        },
                {
            "running": "",
            "alert": "",
            "name": "",
            "host": "",
            "type": "",
            "format": "",
            "ipad": "",
            "port": ,
            "nic": "",
            "intv": ,
            "times": [
                {
                    "wday": {
                        "start": "",
                        "end": ""
                    },
                    "window": {
                        "start": "",
                        "end": ""
                    }
                }
            ]
        }
    ]
}
```
### 3.3. 필드 설명
- `settings`

|필드명|FORMAT|설명|비고|
|:--:|:--:|:--:|:--|
|name|STRING|거래소명|
|type|STRING|거래소 타입|Future/Option/Spread|
|description|STRING|거래소 설명|
|timezone|STRING|거래소 타임존|/usr/share/zoneinfo 내에서 선택|
|room|NUM|품목 등록 가능 개수|
|logLevel|STRING|로그레벨|DEBUG/PROGRESS/ERROR/MUST|

<br>

- `raw_data`

|필드명|FORMAT|설명|비고|
|:--:|:--:|:--:|:--|
|max_date|NUM|로그 남기는 일수 최대치|
|depth_log|NUM|호가 로그 남길지 여부|1 or 0
|cross_check|NUM|호가역전 체크여부|1 or 0

<br>

- `ports`

|필드명|FORMAT|설명|비고|
|:--:|:--:|:--:|:--|
|running|STRING|프로세스 실행 여부|ON/OFF|
|alerts|STRING|Naver Works 메신저로 알림 신호 발송 여부|ON/OFF|
|name|STRING|Port NAME|
|host|STRING|데이터 송신지 Hostname|
|type|STRING|데이터 타입|M(Master)/S(Settlement)/T(Trade)
|format|STRING|데이터 전문 포맷|OLD/HANA/NEW|
|ipad|STRING|Multicast IP|
|port|NUM|Multicast PORT|
|nic|STRING|수신용 Nic 명칭|
|times|ARRAY|wday: 데이터 정상 수신 요일 <br> window: 데이터 정상 수신 시간대|

<br>

---
