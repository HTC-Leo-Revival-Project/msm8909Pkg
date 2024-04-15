#include <Base.h>
#include <Library/LKEnvLib.h>
#include <Chipset/board.h>
#include <Chipset/baseband.h>
#include <Chipset/smem.h>
#include <Library/QcomTargetBoardLib.h>

enum target_subtype {
	HW_PLATFORM_SUBTYPE_SKUAA = 1,
	HW_PLATFORM_SUBTYPE_SKUF = 2,
	HW_PLATFORM_SUBTYPE_SKUAB = 3,
};

/* Detect the target type */
void target_detect(struct board_data *board)
{
	/*
	* already fill the board->target on board.c
	*/

}

/* Detect the modem type */
void target_baseband_detect(struct board_data *board)
{
	uint32_t platform;
	uint32_t platform_subtype;

	platform         = board->platform;
	platform_subtype = board->platform_subtype;

	/*
	 * Look for platform subtype if present, else
	 * check for platform type to decide on the
	 * baseband type
	 */
	switch(platform_subtype)
	{
	case HW_PLATFORM_SUBTYPE_UNKNOWN:
		break;
	case HW_PLATFORM_SUBTYPE_SKUAA:
		break;
	case HW_PLATFORM_SUBTYPE_SKUF:
		break;
	case HW_PLATFORM_SUBTYPE_SKUAB:
		break;
	default:
		dprintf(CRITICAL, "Platform Subtype : %u is not supported\n", platform_subtype);
		ASSERT(0);
	};

	switch(platform)
	{
	case MSM8610:
	case MSM8110:
	case MSM8210:
	case MSM8810:
	case MSM8612:
	case MSM8212:
	case MSM8812:
	case MSM8510:
	case MSM8512:
		board->baseband = BASEBAND_MSM;
		break;
	default:
		dprintf(CRITICAL, "Platform type: %u is not supported\n", platform);
		ASSERT(0);
	};
}
