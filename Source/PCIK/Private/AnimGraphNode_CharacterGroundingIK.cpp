// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimGraphNode_CharacterGroundingIK.h"

//Title Color!
FLinearColor UAnimGraphNode_CharacterGroundingIK::GetNodeTitleColor() const
{
	return FLinearColor(0, 12, 12, 1);
}

//Node Category
FString UAnimGraphNode_CharacterGroundingIK::GetNodeCategory() const
{
	return FString("Anim Node Category");
}

FText UAnimGraphNode_CharacterGroundingIK::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Character Grounding IK"));
}