#pragma once

#include "pch.h"

namespace Raysterizer
{
	namespace Analysis
	{
		struct PositionAffectedInstruction
		{
			uint32_t position_affected_id;
			std::string debug_line;
		};

		struct IdConstructionNode
		{
			uint32_t result_id;
			std::vector<uint32_t> operand_ids;
			SpvOp op;
		};

		struct IdConstructionTree
		{
			flat_hash_map<uint32_t, IdConstructionNode> result_to_node;
		};

		inline bool IsOpcodeSkippable(SpvOp opcode)
		{
			return opcode == SpvOpName ||
				opcode == SpvOpConstant ||
				opcode == SpvOpExtInstImport ||
				opcode == SpvOpEntryPoint ||
				//opcode == SpvOpVariable ||
				spvOpcodeIsDebug(opcode) ||
				spvOpcodeIsDecoration(opcode) ||
				spvOpcodeGeneratesType(opcode);
		}

		class SPIRVAnalyzer
		{
		public:
			explicit SPIRVAnalyzer(std::vector<uint32_t> spv_);
			Error Run();

			uint32_t GetPositionId() { return position_id; }
			const auto& GetPositionAffectedLines() { return position_affected_lines; }
			const auto& GetSpvOptimized() { return spv; }
			const auto& GetIdConstructionTree() { return id_construction_tree; }
		private:
			std::vector<uint32_t> spv{};
			std::vector<uint32_t> spv_optimized{};
			flat_hash_map<uint32_t, PositionAffectedInstruction> position_affected_lines;
			IdConstructionTree id_construction_tree;

			uint32_t position_id{};
		};
	}
}