using UnrealBuildTool;

public class cringe3_topdown : ModuleRules
{
	public cringe3_topdown(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "NavigationSystem", 
															"AIModule", "Niagara", "EnhancedInput", "RHI", "RenderCore", "Slate", 
															"SlateCore", "MoviePlayer" });
    }
}
