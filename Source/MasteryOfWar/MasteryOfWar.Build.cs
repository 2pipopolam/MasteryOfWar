// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MasteryOfWar : ModuleRules
{
	public MasteryOfWar(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bEnableUndefinedIdentifierWarnings = false;


		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject",
			"Engine", 
			"InputCore", 
			"EnhancedInput", 
			"UMG",
			"Slate",
			"SlateCore"});


	if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            // headers
            PublicIncludePaths.Add("/usr/include");
            
            // path to libs
            PublicSystemLibraryPaths.Add("/usr/lib");
            
            PublicSystemLibraries.Add("sqlite3");
            
            // all possible paths
            RuntimeDependencies.Add("$(ProjectDir)/ThirdParty/sqlite/lib/libsqlite3.so");
            RuntimeDependencies.Add("$(ProjectDir)/ThirdParty/sqlite/lib/libsqlite3.so.0");
            RuntimeDependencies.Add("$(ProjectDir)/ThirdParty/sqlite/lib/libsqlite3.so.0.8.6");
        }
        
        // SQLite
        PublicDefinitions.Add("WITH_SQLITE=1");
    }
}