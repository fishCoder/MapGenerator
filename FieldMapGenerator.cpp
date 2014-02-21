#include "FieldMapGenerator.h"


typedef HeroX::NodeList NodeList;

typedef HeroX::PathFinder_new		PathFinder_new;
typedef HeroX::PathResult_new		PathResult_new;

FieldMapGenerator::FieldMapGenerator():kFieldMapParam(),
                                        kMapLogicTemplateTable(),
                                        kBuildingBlock(),
                                        kLootLotteryData(),
                                        kNpcTemplateTalbe(),
                                        fieldmapGenerator(),
                                        map_obj_logic_obj()
{
<<<<<<< HEAD
    kFieldMapParam.Load("/home/lin/Desktop/Public_FieldMapData.csv");
	kBuildingBlock.Load("/home/lin/Desktop/Public_BuildingBlockInfo.csv");
	kMapLogicTemplateTable.Load("/home/lin/Desktop/Map_LogicData.csv");
	kLootLotteryData.Load("/home/lin/Desktop/Loot_Template.csv");
	kNpcTemplateTalbe.Load("/home/lin/Desktop/Npc_Template.csv");
=======
    kFieldMapParam.Load("/home/flb/Desktop/Public_FieldMapData.csv");
	kBuildingBlock.Load("/home/flb/Desktop/Public_BuildingBlockInfo.csv");
	kMapLogicTemplateTable.Load("/home/flb/Desktop/Map_LogicData.csv");
	kLootLotteryData.Load("/home/flb/Desktop/Loot_Template.csv");
	kNpcTemplateTalbe.Load("/home/flb/Desktop/Npc_Template.csv");
>>>>>>> 8877b10bc23c7871b13677cb9afc00721702ccf6

	map_obj_logic_obj.Init(kLootLotteryData, kMapLogicTemplateTable, kNpcTemplateTalbe);
}

std::string FieldMapGenerator::generateMap(int random_seed,int map_logic_id){
    HeroX::NpcPutPlace::MapObjectList mapObjectList;
    HeroX::FieldMapData mapData;

    map_obj_logic_obj.MakeLootData(map_logic_id, mapObjectList);

    const HeroX::MapLogicTemplate* mapLogic = kMapLogicTemplateTable.GetMapLogicTem(map_logic_id);
//    if(mapLogic==NULL) return -1;

    int map_size_index = mapLogic->MapSizeIndex;
    const HeroX::FieldMapParam* param = kFieldMapParam.GetParam(map_size_index);
//    if (param==NULL) return -1;

    fieldmapGenerator.SetBuildBlockTable(kBuildingBlock);

    fieldmapGenerator.Generate(random_seed, *param, *mapLogic, mapObjectList, mapData);

    std::string buffer;
    mapData.SaveBuffer(buffer);

    return buffer;
}
