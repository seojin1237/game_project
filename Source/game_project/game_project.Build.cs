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
			"GameplayTasks"       // 어빌리티 안에서 쓰는 비동기 작업 (몽타주 재생 후 대기 등)
		});

        PrivateDependencyModuleNames.AddRange(new string[] { });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}