# 최우선 규칙 (꼭 지킬 것)
- 최신 사용자 지시가 이 파일보다 우선한다 (파일이 바뀌거나 없어져도 방금 한 말을 따른다)
- 사용자가 직접 고친 파일(에셋·설정·코드)은 덮어쓰지 않는다. 바꿔야 하면 먼저 물어본다
- 자동 검사·빌드 통과는 재미·연출·미술 승인이 아니다. 보고할 때 "코드·빌드로 확인한 것 / 실제 화면·소리로 본 것 / 확인 못 한 것"을 나눠서 말한다
- 정보 없이 코딩하지 않는다 (관련 파일·에셋 값을 읽고 나서 짠다)
- 사용자 말에 무조건 동의하지 않는다 (문제가 있으면 근거와 함께 말한다)
- 테스트·빌드만 보고 "완료"라고 하지 않는다 (직접 플레이로 본 범위만 확인했다고 쓴다)
- 같은 실수를 반복하지 않는다 (지적받은 내용은 작업 메모에 남긴다)
- 작업 중 새로 정한 것은 `Docs/작업메모.md`에 날짜와 사용자가 한 말을 그대로 적는다. 이 파일에는 오래 갈 규칙만 한두 줄로 올린다

# 프로젝트 개요
- Unreal Engine 5.8, C++ + 블루프린트
- 3인칭 슈터 로그라이트 (웨이브 생존), 1인 개발, 스팀 출시 목표
- C++ 모듈 이름: `game_project` (API 매크로: `GAME_PROJECT_API`)

# 나에 대해
- C++ 공부 중 (포인터까지 학습함), 언리얼 학원 수강 중
- C# / Unity 경험 있음 → 개념 설명 시 C#과 비교해주면 좋음
- 직접 해보면서 배우는 스타일

# 작업 규칙
- 답변과 코드 주석은 한국어로
- 코드를 작성하면 왜 그렇게 짰는지 핵심 개념을 짧게 설명할 것
- 한 번에 큰 기능을 다 만들지 말고 작은 단계로 나눠서 진행
- 로직은 C++, 연출·수치 튜닝은 블루프린트에서 하도록 설계
- 코드를 채팅에 보여주는 범위 (토큰 절약을 위해 요약해서 어떤 코드를 사용했고 어떤점이 변했는지 간단하게 보여주기)
- 세션은 난이도별로 나눠 쓴다: Opus 5.5 세션 = 복잡한 작업(새 기능, 원인 모를 버그, 구조 변경), Sonnet 5.5 세션 = 간단한 작업(수치 조절, 에셋 값 변경, 커밋). 세션 중에는 모델을 바꾸지 않는다 (프롬프트 캐시가 모델별로 분리되어 토큰이 낭비됨)
  - 사용자는 평소 Sonnet 세션에만 말한다. 복잡한 작업은 Sonnet 세션이 세션 간 메시지로 Opus 세션에 넘기고 사용자에게 알린다
  - Opus 세션은 끝나면 Sonnet 세션에 완료 신호, 커밋 대상 파일, 확인한 범위를 보낸다

# 사용 시스템
- **Enhanced Input**: 모든 입력은 Input Action(IA_) + Input Mapping Context(IMC_)로 처리
- **GAS (Gameplay Ability System)**: 스킬·무기 발사·체력/데미지·버프·업그레이드에 사용
  - 사용자가 GAS를 처음 배우는 중 → GAS 관련 코드는 개념(ASC / AttributeSet / Ability / Effect / Tag / Cue) 중 어디에 해당하는지 먼저 설명하고 진행
  - 수치(데미지, 쿨타임, 업그레이드 값)는 C++ 하드코딩 대신 GameplayEffect / DataTable / Curve로 BP에서 조절 가능하게

# 빌드 방식
- **Live Coding 사용 안 함**
- **헤더(.h) 변경 시** (UPROPERTY/UFUNCTION 추가·삭제, 새 클래스, 함수 선언 변경): 에디터 종료 → IDE에서 `Development Editor` / `Win64`로 빌드 → 에디터 재실행
  - 에디터를 켠 채 빌드하면 Hot Reload가 일어나 블루프린트 값이 날아가거나 에셋이 깨질 수 있음
- **.cpp만 변경 시**: 에디터 켜둔 채 IDE에서 빌드(Ctrl+Shift+B)해도 됨. 동작이 이상하면 에디터 재시작
- 코드 수정을 안내할 때마다 이번 변경이 위 둘 중 어느 쪽인지(에디터를 꺼야 하는지) 명시할 것
- 빌드 에러 설명 시 에러 메시지의 핵심 줄을 짚어서 원인을 설명할 것

# 이름 규칙 (Epic 코딩 표준 + 대중적인 UE 에셋 네이밍 가이드 기준)

## C++
| 대상 | 접두사 | 예시 |
|---|---|---|
| AActor 파생 | `A` | `AShooterCharacter`, `AEnemyBase` |
| UObject / UActorComponent 파생 | `U` | `UHealthComponent`, `UShooterAttributeSet` |
| 구조체 | `F` | `FWaveInfo` |
| 열거형 | `E` | `EWeaponType` (`enum class EWeaponType : uint8`) |
| 인터페이스 | `I` | `IInteractable` |
| 템플릿 | `T` | `TArray`, `TObjectPtr` |
| bool 변수 | `b` | `bIsDead`, `bCanFire` |

- 클래스·함수·변수 모두 PascalCase (`CurrentWave`, `StartWave()`)
- 함수 매개변수 앞에 `In`/`Out` 사용 가능 (`InDamage`, `OutHitResult`)
- 헤더의 UObject 포인터 멤버는 `TObjectPtr<>` 사용

## 에셋
| 종류 | 접두사 | 종류 | 접두사 |
|---|---|---|---|
| 블루프린트 | `BP_` | 위젯 블루프린트 | `WBP_` |
| 애니메이션 블루프린트 | `ABP_` | 애님 몽타주 | `AM_` |
| 애니메이션 시퀀스 | `A_` | 블렌드 스페이스 | `BS_` |
| 스태틱 메시 | `SM_` | 스켈레탈 메시 | `SK_` |
| 머티리얼 | `M_` | 머티리얼 인스턴스 | `MI_` |
| 텍스처 | `T_` | 나이아가라 시스템 | `NS_` |
| 사운드 큐 / 웨이브 | `SC_` / `S_` | 데이터 에셋 | `DA_` |
| 데이터 테이블 | `DT_` | 커브 | `Curve_` |
| Input Action | `IA_` | Input Mapping Context | `IMC_` |
| Gameplay Ability | `GA_` | Gameplay Effect | `GE_` |
| Gameplay Cue | `GC_` | 레벨(맵) | `L_` |

- 태그는 계층형으로: `Ability.Weapon.Fire`, `State.Dead`, `Data.Damage`

# 버전 관리
- Git 사용 (`.gitignore`, `.gitattributes` 있음)
- `.uasset` / `.umap` 등 바이너리는 Git LFS로 관리
- 원격 저장소: `origin` = `https://github.com/seojin1237/game_project.git` (`main` 브랜치)
- **커밋·푸시는 항상 Sonnet 세션에서만** 한다. Opus 세션은 커밋하지 않는다. 두 세션이 함께 한 작업은 둘 다 끝난 뒤 한 번에 커밋한다
  - 이번 작업 파일만 골라서 add 한다 (관련 없는 변경은 지시 없이 넣지 않는다). 에디터에서 바꾼 에셋은 저장돼 있어야 한다
  - 커밋 메시지에는 직접 확인한 범위만 적는다 (직접 플레이 전이면 "동작 확인 완료"라고 쓰지 않는다)
- **기능 하나를 마칠 때마다 커밋하고 바로 `git push origin main`까지 할 것** (백업 목적, 사용자 승인됨)
  - 중간 단계 커밋은 로컬에만 두고, 기능이 동작 확인되면 푸시
  - 푸시가 인증 문제로 실패하면 터미널 패널에서 `git push origin main`을 실행해 사용자가 브라우저 인증을 하도록 안내
