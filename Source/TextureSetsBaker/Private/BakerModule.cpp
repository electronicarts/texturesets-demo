// Copyright (c) 2025 Electronic Arts. All Rights Reserved.

#include "BakerModule.h"

#include "TextureSetProcessingGraph.h"
#include "TextureSetSampleFunctionBuilder.h"
#include "TextureSetDefinition.h"
#include "TextureSet.h"
#include "TextureSetDefinition.h"
#include "BakeUtil.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshSourceData.h"
#include "StaticMeshCompiler.h"
#include "MeshDescription.h"
#include "ProcessingNodes/IProcessingNode.h"

void UBakerAssetParams::FixupData(UObject* Outer)
{
	// Update baker params to match the baker modules in the definition

	UTextureSet* TextureSet = CastChecked<UTextureSet>(Outer);
	UTextureSetDefinition* Definition = TextureSet->Definition;
	
	if (IsValid(Definition))
	{
		TMap<FName, FBakerInstanceParam> OldBakerParams = BakerParams;
		BakerParams.Empty(OldBakerParams.Num());

		for (const UTextureSetModule* Module : Definition->GetModuleInfo().GetModules())
		{
			const UBakerModule* BakerModule = Cast<UBakerModule>(Module);
			if (IsValid(BakerModule))
			{
				if (OldBakerParams.Contains(BakerModule->ElementName))
				{
					BakerParams.Add(BakerModule->ElementName, OldBakerParams.FindChecked(BakerModule->ElementName));
				}
				else
				{
					BakerParams.Add(BakerModule->ElementName, FBakerInstanceParam());
				}
			}

		}
	}
}

class FTextureBaker : public ITextureProcessingNode
{
public:
	FTextureBaker(FName Name) : ITextureProcessingNode()
		, Name(Name)
		, bPrepared(false)
		, BakeArgs()
		, BakeResults()
	{
	}

	virtual FName GetNodeTypeName() const  { return "TextureBake"; }

	virtual void ComputeGraphHash(FHashBuilder& HashBuilder) const override
	{
		HashBuilder << Name;
		HashBuilder << GetNodeTypeName();
	}

	virtual void ComputeDataHash(const FTextureSetProcessingContext& Context, FHashBuilder& HashBuilder) const override
	{
		const UBakerAssetParams* BakerModuleAssetParams = Context.AssetParams.Get<UBakerAssetParams>();
		const FBakerInstanceParam* BakerParams = BakerModuleAssetParams->BakerParams.Find(Name);

		HashBuilder << BakerParams->BakedTextureWidth;
		HashBuilder << BakerParams->BakedTextureHeight;

		if (IsValid(BakerParams->SourceMesh))
		{
			FStaticMeshSourceModel* Model = &BakerParams->SourceMesh->GetSourceModel(0);
			HashBuilder << Model->StaticMeshDescriptionBulkData->GetBulkData().GetIdString();
		}
	}

	virtual void Prepare(const FTextureSetProcessingContext& Context) override
	{
		if (bPrepared)
			return;

		const UBakerAssetParams* BakerModuleAssetParams = Context.AssetParams.Get<UBakerAssetParams>();
		const FBakerInstanceParam* BakerParams = BakerModuleAssetParams->BakerParams.Find(Name);
		if (BakerParams->SourceMesh)
		{
			BakeArgs.BakeWidth = BakerParams->BakedTextureWidth;
			BakeArgs.BakeHeight = BakerParams->BakedTextureHeight;

			if (BakerParams->SourceMesh->IsCompiling())
			{
				FStaticMeshCompilingManager::Get().FinishCompilation({ BakerParams->SourceMesh });
			}

			if (IsValid(BakerParams->SourceMesh))
			{
				SourceModel = &BakerParams->SourceMesh->GetSourceModel(0);
			}
		}
		bPrepared = true;
	}

	virtual void Cache() override
	{
		// TODO: Cache the result in the DDC
		if (SourceModel)
		{
			SourceModel->LoadRawMesh(BakeArgs.RawMesh);
			BakeResults.Pixels.SetNumUninitialized(BakeArgs.BakeWidth * BakeArgs.BakeHeight);
			FBakeUtil::BakeUV(BakeArgs, BakeResults);
		}
	}

	virtual const FTextureSetProcessedTextureDef GetTextureDef() const override
	{
		return FTextureSetProcessedTextureDef(
			1,
			ETextureSetChannelEncoding::RangeCompression,
			ETextureSetTextureFlags::Default);
	}

	virtual FTextureDimension GetTextureDimension() const override
	{
		check(bPrepared);
		FTextureDimension Dim;
		Dim.Width = BakeArgs.BakeWidth;
		Dim.Height = BakeArgs.BakeWidth;
		Dim.Slices = 1;
		return Dim;
	}

	void WriteChannel(int32 Channel, const FTextureDataTileDesc& Tile, float* TextureData) const override
	{
		if (BakeResults.Pixels.Num() > 0)
		{
			Tile.ForEachPixel([TextureData, Channel, &Tile, this](FTextureDataTileDesc::ForEachPixelContext& Context)
			{
				const FIntVector3 TextureCoord = Tile.TileOffset + Context.TileCoord;
				const int32 TextureDataIndex = (TextureCoord.X % BakeArgs.BakeWidth) + (TextureCoord.Y * BakeArgs.BakeWidth);
				TextureData[Context.DataIndex] = BakeResults.Pixels[TextureDataIndex];
			});
		}
		else
		{
			// Fill tile data with default value
			Tile.ForEachPixel([TextureData, Channel, this](FTextureDataTileDesc::ForEachPixelContext& Context)
			{
				TextureData[Context.DataIndex] = 0.5f;
			});
		}
	}

private:
	FName Name;
	bool bPrepared;
	FStaticMeshSourceModel* SourceModel = nullptr;
	FBakeUtil::FBakeArgs BakeArgs;
	FBakeUtil::FBakeResults BakeResults;
};

void UBakerModule::ConfigureProcessingGraph(FTextureSetProcessingGraph& Graph) const
{
	Graph.AddOutputTexture(ElementName, MakeShared<FTextureBaker>(ElementName));
}

void UBakerModule::ConfigureSamplingGraphBuilder(const FTextureSetAssetParamsCollection* SampleParams,
	FTextureSetSampleFunctionBuilder* Builder) const
{
	Builder->AddSubsampleFunction(ConfigureSubsampleFunction([this, Builder](FTextureSetSubsampleBuilder& Subsample)
	{
		// Simply create a sample result for the element
		Subsample.AddResult(ElementName, Subsample.GetSharedValue(ElementName));
	}));
}
