/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   articulations_back_png;
    const int            articulations_back_pngSize = 147592;

    extern const char*   button_small_off_png;
    const int            button_small_off_pngSize = 10272;

    extern const char*   button_small_on_png;
    const int            button_small_on_pngSize = 9866;

    extern const char*   menu_background_png;
    const int            menu_background_pngSize = 18892;

    extern const char*   knob_big1_png;
    const int            knob_big1_pngSize = 46635591;

    extern const char*   new_background_png;
    const int            new_background_pngSize = 1220763;

    extern const char*   Super_Vanilla_ttf;
    const int            Super_Vanilla_ttfSize = 225560;

    extern const char*   ASPIRE_AUDIO_png;
    const int            ASPIRE_AUDIO_pngSize = 3776;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 8;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
