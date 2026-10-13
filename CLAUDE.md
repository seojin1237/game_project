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
- UPROPERTY / UFUNCTION 지정자를 쓸 때는 각각 의미를 주석으로 달 것
- 한 번에 큰 기능을 다 만들지 말고 작은 단계로 나눠서 진행
- 로직은 C++, 연출·수치 튜닝은 블루프린트에서 하도록 설계
- 파일 수정 시 부분이 아닌 전체 파일 내용을 보여줄 것

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
