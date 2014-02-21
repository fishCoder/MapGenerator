#include <FieldMapData.h>
#include <ValuePointGenerator.h>
#include <MiniTreeGenerator.h>
#include <PathFinder_new.h>
#include <time.h>
#include <FieldMapGenerator.h>
#include <NpcPutPlace.h>
#include <MapLogicDef.h>
//#include <TY/Common/BuildingBlockDef.h>
#include <TreeNode.h>
#include <RandomGenerator.h>

#include <MapObjectLogic.h>
#include <LootLotteryMgr.h>

class FieldMapGenerator{
public:
    FieldMapGenerator();
    std::string generateMap(int random_seed,int map_logic_id);
protected:
private:
	HeroX::FieldMapParamTable		kFieldMapParam;
	HeroX::MapLogicTemplateTable			kMapLogicTemplateTable;
	HeroX::BuildingBlockTable		kBuildingBlock;
	HeroX::LootLotteryMgr		kLootLotteryData;
	HeroX::NpcBaseTemplateTable kNpcTemplateTalbe;

    HeroX::FieldMapGenerator fieldmapGenerator;
	HeroX::MapObjectLogic  map_obj_logic_obj;
};
