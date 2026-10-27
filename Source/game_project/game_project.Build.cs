// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class game_project : ModuleRules
{
    public game_project(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",      // Enhanced Input (IA_ / IMC_)

			// ===== GAS =====
			"GameplayAbilities",  // ASC, AttributeSet, GameplayAbility, GameplayEffect
			"GameplayTags",       // FGameplayTag (State.Dead 같은 계층형 태그)
			"GameplayTasks",      // 어빌리티 안에서 쓰는 비동기 작업 (몽타주 재생 후 대기 등)

			// ===== AI =====
			"AIModule",           // AAIController, MoveToActor, Behavior Tree 등 AI 기본 기능

			// ===== UI =====
			"UMG"                 // UUserWidget, UProgressBar, UTextBlock 등 위젯(WBP_) 기능
		});

        // UMG 내부가 사용하는 저수준 UI 프레임워크 (헤더에 노출하지 않으므로 Private)
        PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Niagara 이펙트를 C++에서 스폰 (총구 화염). 헤더에는 전방 선언만 쓰므로 Private
        PrivateDependencyModuleNames.Add("Niagara");

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}