# Daisy Seed 기타 딜레이 펌웨어

Daisy Seed 또는 Seed3 모듈용 STM32H750 펌웨어. 외부 SDRAM이 탑재된 Seed가 필요하다. 온보드 코덱과 SAI DMA로 48 kHz 오디오를 48프레임씩 처리한다. 기타 신호는 왼쪽 입력으로 받고 같은 모노 신호를 좌우 출력에 보낸다.

## 업로드

아래 빌드 절차에서 생성한 `build/daisy_delay.bin`은 내부 플래시 주소 `0x08000000`용이다.

1. 데이터 USB 케이블을 연결한다. BOOT을 누른 상태에서 RESET을 눌렀다 떼고 BOOT을 놓는다.
2. [Daisy Programmer](https://flash.daisy.audio/)의 **File Upload**에서 `daisy_delay.bin`을 올린 뒤 **Flash**를 누른다. USB 장치 선택 창에서는 STM32의 DFU 장치를 선택한다.
3. 업로드 후 보드가 자동으로 재시작하지 않으면 RESET을 한 번 누른다. 시작 상태는 DSP 바이패스이고, Bypass 스위치를 한 번 누르면 내장 LED와 딜레이가 켜진다.

`dfu-util`을 쓰는 경우에는 같은 DFU 모드에서 저장소 루트에서 아래 명령을 실행한다.

```sh
dfu-util -d 0483:df11 -a 0 -s 0x08000000:leave -D build/daisy_delay.bin
```

이 주소에 이미 Daisy의 선택형 부트로더나 다른 프로그램이 있다면 업로드가 이를 덮어쓴다.

## 단자와 조작

핀 이름은 모듈에 표시된 논리 핀 기준이다. 아래 배선은 **Seed 모듈 단독**에 맞춘 것이다. 노브는 10 kΩ 선형 포텐셔미터 기준으로 양 끝을 3.3 V와 GND에, 와이퍼를 ADC 핀에 연결한다. 스위치 입력은 내부 풀업을 사용한다.

| 조작부 | Daisy 핀 | 동작 |
| --- | --- | --- |
| Time | A0 / D15 | 20–2000 ms. 짧은 구간을 세밀하게 조정할 수 있게 제곱 곡선을 사용한다. |
| Mix | A1 / D16 | Dry 100%에서 Wet 100%까지 선형 크로스페이드한다. |
| Feedback | A2 / D17 | 반복을 조절한다. 피드백 계수 상한은 0.94다. |
| Tone | A3 / D18 | 반복음에 약 700 Hz–9 kHz 범위의 로우패스를 적용한다. Dry 신호에는 이 필터를 걸지 않는다. |
| Bypass | D0 ↔ GND, 모멘터리 | 한 번 누를 때마다 효과를 켜거나 끈다. 전환은 약 10 ms에 걸쳐 이루어진다. |
| Tap | D1 ↔ GND, 모멘터리 | 두 번 이상 탭하면 4분음표 기준 딜레이 시간이 설정된다. |
| Mode | D2 ↔ GND, 래칭 | 열림은 크로스페이드, GND에 연결되면 피치 글라이드다. |

탭 템포는 150–2000 ms 간격을 받아 최근 최대 세 간격을 평균한다. 2초보다 오래 쉬면 다음 탭부터 새 템포를 잰다. 탭으로 설정한 시간은 Time 노브를 약 2.5% 움직일 때까지 유지된다. 각 스위치에는 8 ms 디바운스를 적용한다.

Bypass는 ADC와 DAC를 통과하는 DSP 바이패스다. 바이패스로 전환하면 반복음 출력과 딜레이 입력이 약 10 ms에 걸쳐 줄어들고, 버퍼 안의 반복음은 내부에서 계속 감쇠한다. 릴레이와 기타용 고입력 임피던스 버퍼, 입출력 레벨 조정은 외부 회로 쪽이다. 기타 잭은 적절한 버퍼와 레벨 조정을 거쳐 왼쪽 입력에 연결한다. 모듈의 오디오·전원 핀 위치는 [Daisy Seed](https://docs.daisy.audio/hardware/Seed/) 또는 [Seed3](https://docs.daisy.audio/hardware/Seed3/) 핀아웃을 따른다.

## 시간 변경 방식

- **크로스페이드**. 기존 시간과 새 시간에서 읽은 반복음을 25 ms 동안 교차시킨다. 350→500 ms로 바꿀 때 읽기 위치를 빠르게 미끄러뜨려 생기는 연속 피치 변화를 피한다.
- **피치 글라이드**. 한 읽기 위치의 지연 시간을 샘플마다 최대 0.25샘플씩 이동시킨다. 350→500 ms는 약 600 ms 동안 피치를 낮추면서 이동한다. 반대로 시간을 줄이면 피치가 올라간다.

둘 다 분수 샘플의 읽기에는 선형 보간을 쓴다. 링 버퍼는 외부 SDRAM 384,008 B를 사용하고, 오디오 콜백에서는 추가 메모리 할당을 하지 않는다.

## 다시 빌드하기

GNU Make, Git, Arm GNU Toolchain(`arm-none-eabi-gcc` 및 `arm-none-eabi-g++`)을 PATH에 둔다. 초기 버전은 Arm GNU Toolchain 13.2.Rel1과 libDaisy v9.0.0으로 빌드했다.

```sh
git clone --branch v9.0.0 --recurse-submodules https://github.com/daisyaudio/libDaisy.git
make -C libDaisy -j4
make -j4
```

빌드 결과는 `build/daisy_delay.bin`, `build/daisy_delay.elf`, `build/daisy_delay.hex`다. 로컬 빌드 결과의 DFU 업로드는 `make program-dfu`로도 가능하다.

- `src/main.cpp`는 핀 연결과 오디오 콜백을 연결한다.
- `src/delay.cpp`는 딜레이 버퍼, 반복음 필터와 두 가지 시간 전환을 처리한다.
- `src/controls.cpp`는 노브, 스위치와 탭 템포를 처리한다.

## 보드 없이 회귀 테스트 실행

```sh
make -C tests check
```

호스트 C++ 컴파일러로 반복음 타이밍과 버퍼 순환, 시간 전환, 바이패스, 풋스위치 디바운스와 탭 템포를 검증한다. 실제 Seed에서의 플래시, 오디오 레벨과 노이즈, DMA 실시간 동작은 보드를 연결한 뒤 확인한다.
