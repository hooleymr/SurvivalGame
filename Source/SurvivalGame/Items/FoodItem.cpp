// Fill out your copyright notice in the Description page of Project Settings.


#include "FoodItem.h"

#define LOCTEXT_NAMESPACE "FoodItem"

UFoodItem::UFoodItem()
{
	HealAmount = 20.f;
	UseActionText = LOCTEXT("ItemUseActionText", "Consume");

}

void UFoodItem::Use(ASurvivalCharacter * Character)
{
	//heal character here
}


#undef LOCTEXT_NAMESPACE