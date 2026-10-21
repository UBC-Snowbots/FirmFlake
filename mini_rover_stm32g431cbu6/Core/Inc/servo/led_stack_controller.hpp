// #pragma once
// #include "led_panel_interface.hpp"
// #include <stdint.h>

// using enum LedPanelStatus;

// // Panel type must be passed, c++ needs to know the size in order to allocate the array
// template <typename PanelT, uint32_t num_panels_in_stack>
// class LedStackController {
// public:
//     LedStackController(PanelT (&panels)[num_panels_in_stack])
//     : panels(panels) {};

//     LedPanelStatus init_all(uint32_t& failed_index)
//     {
//         for(auto &panel : panels)
//         {
//             if(panel.init() != OK)
//             {
//                 failed_index = panel.get_index();
//                 return NOT_OK;
//             }
//         }
//         return LedPanelStatus::OK;
//     }
//     LedPanelStatus turn_all_off(void)
//     {
//         for(auto &panel : panels)
//         {
//             if(panel.set_duty(0.00) != OK)
//             {
//                 return NOT_OK;
//             }
//         }

//         return OK;
//     }

//     PanelT& operator[](int i) { 
//         if(i < 0)
//         {
//             return panels[0];
//         }
//         if(i >= NUM_LED_PANELS)
//         {
//             return panels[NUM_LED_PANELS - 1]; 

//         }
//         return panels[i]; 

//     }
//     const PanelT& operator[](int i) const { 
//         if(i < 0)
//         {
//             return panels[0];
//         }
//         if(i >= NUM_LED_PANELS)
//         {
//             return panels[NUM_LED_PANELS - 1]; 

//         }
//         return panels[i]; 

//     }

//     // I was lazy and made this public
//     PanelT (&panels)[num_panels_in_stack];
//     private:

// };
