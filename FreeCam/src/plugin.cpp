#include "plugin.h"

#include <JoJoAPI.h>

JAPIModMeta GetModMeta() {
    static JAPIModMeta meta = {
        "FreeCam",
        "Kapilarny & Kojo Bailey",
        "FreeCam",
        "1.2.0",
        "Frees the camera."
    };

    return meta;
}

void ModInit() {
    JINFO("FreeCam initialized!");
}