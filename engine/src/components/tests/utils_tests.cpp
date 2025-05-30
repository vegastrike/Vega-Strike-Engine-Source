#include <gtest/gtest.h>
#include <string>

#include "components/component_utils.h" 

std::string dummy_color(double percent) {
    if(percent == 1.0) {
        return "white:";
    } else if(percent == 0.0) {
        return "grey:";
    } else if(percent > 0.66) {
        return "yellow:";
    } else if(percent > 0.33) {
        return "orange:";
    } else {
        return "red:";
    } 
}


TEST(UtilsTests, PrintFormattedComponentInHud) {
    double percents[] = {1.0, 0.0, 0.8, 0.6, 0.4, 0.2, 0.1};

    std::string expected_outputs[] = {"white:Dummy (100%)white:\n",
                                      "grey:Dummy (0%)white:\n",
                                      "yellow:Dummy (80%)white:\n",
                                      "orange:Dummy (60%)white:\n",
                                      "orange:Dummy (40%)white:\n",
                                      "red:Dummy (20%)white:\n",
                                      "red:Dummy (10%)white:\n"};

    for(int i=0;i<7;i++) {
        std::string output = PrintFormattedComponentInHud(percents[i], "Dummy", true, dummy_color);
        EXPECT_EQ(output, expected_outputs[i]);
    }
}