#pragma once

// You should place include statements to your module's private header files here.  You only need to
// add includes for headers that are used in most of your module's source files though.

#include "Logging/StructuredLog.h"

#include "Core.h"
#include "Engine/World.h"

#include "Kismet/GameplayStatics.h"
#include "GuessExStatics.h"

#include "CoreEx.h"


//General Log
DECLARE_LOG_CATEGORY_EXTERN(LogStateMachineEx, Log, All);

//Logging during module startup
//DECLARE_LOG_CATEGORY_EXTERN(LogStateMachineExInit, Log, All);

//Logging for Critical Errors that must always be addressed
DECLARE_LOG_CATEGORY_EXTERN(LogStateMachineExCriticalErrors, Log, All);


#include "StateMachineEx.h"
