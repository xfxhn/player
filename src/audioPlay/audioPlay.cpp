
#include "audioPlay.h"
#include "CAudioPlay.h"

AudioPlay *AudioPlay::get() {

    static CAudioPlay play;
    return &play;
}



AudioPlay::~AudioPlay() = default;
