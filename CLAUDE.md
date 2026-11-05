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
- 코드를 작성하면 왜 그렇게 짰는지 핵심 개념을 한두 줄로 설명할 것
- UPROPERTY / UFUNCTION 지정자를 쓸 때는 각각 의미를 주석으로 달 것
- 코드 주석은 자세하게: 함수/블록마다 "무엇을 하는지"와 "왜 이렇게 하는지"를 적고, 처음 나오는 언리얼 API·개념은 한 줄 설명(가능하면 C#/Unity 비유)을 붙일 것
- 한 번에 큰 기능을 다 만들지 말고 작은 단계로 나눠서 진행
- 로직은 C++, 연출·수치 튜닝은 블루프린트에서 하도록 설계
- 코드는 사용자가 요청할 때만 채팅에 보여줄 것 (새 파일 포함). 평소에는 파일 링크만 달 것
- 작업 보고는 짧게: 무엇이 됐는지, 확인 방법, 조절할 값의 표(조절할 것 / 위치 / 현재 값) 정도로 끝낼 것

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
- **기능 하나를 마칠 때마다 커밋하고 바로 `git push origin main`까지 할 것** (백업 목적, 사용자 승인됨)
  - 중간 단계 커밋은 로컬에만 두고, 기능이 동작 확인되면 푸시
  - 푸시가 인증 문제로 실패하면 터미널 패널에서 `git push origin main`을 실행해 사용자가 브라우저 인증을 하도록 안내
