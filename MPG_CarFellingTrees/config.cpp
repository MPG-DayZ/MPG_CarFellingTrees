class CfgPatches {
  class MPG_CarFellingTrees {
    units[] = {};
    weapons[] = {};
    requiredVersion = 0.1;
    requiredAddons[] = { "DZ_Data", "DZ_Scripts", "DZ_Vehicles_Wheeled" };
  };
};

class CfgMods {
  class MPG_CarFellingTrees {
    type = "mod";
    author = "pafnuty";
    authorID = "76561198022444951";
    version = "1.0";
    dependencies[] = { "World" };

    class defs {
      class worldScriptModule {
        value = "";
        files[] = { "MPG_CarFellingTrees/Scripts/4_World" };
      };
    };
  };
};
