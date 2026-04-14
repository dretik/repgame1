#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CPP_VisualTextInterface.generated.h"

UINTERFACE(MinimalAPI)
class UCPP_VisualTextInterface : public UInterface { GENERATED_BODY() };

class PROJBLANK_API ICPP_VisualTextInterface
{
    GENERATED_BODY()

public:
    //func for text actors
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Visuals")
        void SetValue(float Value);
};