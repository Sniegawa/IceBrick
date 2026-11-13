#include "Core/WindowInput.hpp"

namespace IceBrick
{
    // Thanks Gemini :)
    [[nodiscard]] Key MapGlfwKey(int glfwKey) noexcept 
    {
        switch (glfwKey) {
        // Printable keys
        case 32 /*GLFW_KEY_SPACE*/:         return Key::Space;
        case 39 /*GLFW_KEY_APOSTROPHE*/:    return Key::Apostrophe;
        case 44 /*GLFW_KEY_COMMA*/:         return Key::Comma;
        case 45 /*GLFW_KEY_MINUS*/:         return Key::Minus;
        case 46 /*GLFW_KEY_PERIOD*/:        return Key::Period;
        case 47 /*GLFW_KEY_SLASH*/:         return Key::Slash;
        case 48 /*GLFW_KEY_0*/:             return Key::Num0;
        case 49 /*GLFW_KEY_1*/:             return Key::Num1;
        case 50 /*GLFW_KEY_2*/:             return Key::Num2;
        case 51 /*GLFW_KEY_3*/:             return Key::Num3;
        case 52 /*GLFW_KEY_4*/:             return Key::Num4;
        case 53 /*GLFW_KEY_5*/:             return Key::Num5;
        case 54 /*GLFW_KEY_6*/:             return Key::Num6;
        case 55 /*GLFW_KEY_7*/:             return Key::Num7;
        case 56 /*GLFW_KEY_8*/:             return Key::Num8;
        case 57 /*GLFW_KEY_9*/:             return Key::Num9;
        case 59 /*GLFW_KEY_SEMICOLON*/:     return Key::Semicolon;
        case 61 /*GLFW_KEY_EQUAL*/:         return Key::Equal;
        case 65 /*GLFW_KEY_A*/:             return Key::A;
        case 66 /*GLFW_KEY_B*/:             return Key::B;
        case 67 /*GLFW_KEY_C*/:             return Key::C;
        case 68 /*GLFW_KEY_D*/:             return Key::D;
        case 69 /*GLFW_KEY_E*/:             return Key::E;
        case 70 /*GLFW_KEY_F*/:             return Key::F;
        case 71 /*GLFW_KEY_G*/:             return Key::G;
        case 72 /*GLFW_KEY_H*/:             return Key::H;
        case 73 /*GLFW_KEY_I*/:             return Key::I;
        case 74 /*GLFW_KEY_J*/:             return Key::J;
        case 75 /*GLFW_KEY_K*/:             return Key::K;
        case 76 /*GLFW_KEY_L*/:             return Key::L;
        case 77 /*GLFW_KEY_M*/:             return Key::M;
        case 78 /*GLFW_KEY_N*/:             return Key::N;
        case 79 /*GLFW_KEY_O*/:             return Key::O;
        case 80 /*GLFW_KEY_P*/:             return Key::P;
        case 81 /*GLFW_KEY_Q*/:             return Key::Q;
        case 82 /*GLFW_KEY_R*/:             return Key::R;
        case 83 /*GLFW_KEY_S*/:             return Key::S;
        case 84 /*GLFW_KEY_T*/:             return Key::T;
        case 85 /*GLFW_KEY_U*/:             return Key::U;
        case 86 /*GLFW_KEY_V*/:             return Key::V;
        case 87 /*GLFW_KEY_W*/:             return Key::W;
        case 88 /*GLFW_KEY_X*/:             return Key::X;
        case 89 /*GLFW_KEY_Y*/:             return Key::Y;
        case 90 /*GLFW_KEY_Z*/:             return Key::Z;
        case 91 /*GLFW_KEY_LEFT_BRACKET*/:  return Key::LeftBracket;
        case 92 /*GLFW_KEY_BACKSLASH*/:     return Key::Backslash;
        case 93 /*GLFW_KEY_RIGHT_BRACKET*/: return Key::RightBracket;
        case 96 /*GLFW_KEY_GRAVE_ACCENT*/:  return Key::GraveAccent;

        // Function keys
        case 256 /*GLFW_KEY_ESCAPE*/:       return Key::Escape;
        case 257 /*GLFW_KEY_ENTER*/:        return Key::Enter;
        case 258 /*GLFW_KEY_TAB*/:          return Key::Tab;
        case 259 /*GLFW_KEY_BACKSPACE*/:    return Key::Backspace;
        case 260 /*GLFW_KEY_INSERT*/:       return Key::Insert;
        case 261 /*GLFW_KEY_DELETE*/:       return Key::Delete;
        case 262 /*GLFW_KEY_RIGHT*/:        return Key::Right;
        case 263 /*GLFW_KEY_LEFT*/:         return Key::Left;
        case 264 /*GLFW_KEY_DOWN*/:         return Key::Down;
        case 265 /*GLFW_KEY_UP*/:           return Key::Up;
        case 266 /*GLFW_KEY_PAGE_UP*/:      return Key::PageUp;
        case 267 /*GLFW_KEY_PAGE_DOWN*/:    return Key::PageDown;
        case 268 /*GLFW_KEY_HOME*/:         return Key::Home;
        case 269 /*GLFW_KEY_END*/:          return Key::End;
        case 280 /*GLFW_KEY_CAPS_LOCK*/:    return Key::CapsLock;
        case 281 /*GLFW_KEY_SCROLL_LOCK*/:  return Key::ScrollLock;
        case 282 /*GLFW_KEY_NUM_LOCK*/:     return Key::NumLock;
        case 283 /*GLFW_KEY_PRINT_SCREEN*/: return Key::PrintScreen;
        case 284 /*GLFW_KEY_PAUSE*/:        return Key::Pause;
        
        case 290 /*GLFW_KEY_F1*/:           return Key::F1;
        case 291 /*GLFW_KEY_F2*/:           return Key::F2;
        case 292 /*GLFW_KEY_F3*/:           return Key::F3;
        case 293 /*GLFW_KEY_F4*/:           return Key::F4;
        case 294 /*GLFW_KEY_F5*/:           return Key::F5;
        case 295 /*GLFW_KEY_F6*/:           return Key::F6;
        case 296 /*GLFW_KEY_F7*/:           return Key::F7;
        case 297 /*GLFW_KEY_F8*/:           return Key::F8;
        case 298 /*GLFW_KEY_F9*/:           return Key::F9;
        case 299 /*GLFW_KEY_F10*/:          return Key::F10;
        case 300 /*GLFW_KEY_F11*/:          return Key::F11;
        case 301 /*GLFW_KEY_F12*/:          return Key::F12;

        // Keypad
        case 320 /*GLFW_KEY_KP_0*/:         return Key::Kp0;
        case 321 /*GLFW_KEY_KP_1*/:         return Key::Kp1;
        case 322 /*GLFW_KEY_KP_2*/:         return Key::Kp2;
        case 323 /*GLFW_KEY_KP_3*/:         return Key::Kp3;
        case 324 /*GLFW_KEY_KP_4*/:         return Key::Kp4;
        case 325 /*GLFW_KEY_KP_5*/:         return Key::Kp5;
        case 326 /*GLFW_KEY_KP_6*/:         return Key::Kp6;
        case 327 /*GLFW_KEY_KP_7*/:         return Key::Kp7;
        case 328 /*GLFW_KEY_KP_8*/:         return Key::Kp8;
        case 329 /*GLFW_KEY_KP_9*/:         return Key::Kp9;
        case 330 /*GLFW_KEY_KP_DECIMAL*/:   return Key::KpDecimal;
        case 331 /*GLFW_KEY_KP_DIVIDE*/:    return Key::KpDivide;
        case 332 /*GLFW_KEY_KP_MULTIPLY*/:  return Key::KpMultiply;
        case 333 /*GLFW_KEY_KP_SUBTRACT*/:  return Key::KpSubtract;
        case 334 /*GLFW_KEY_KP_ADD*/:       return Key::KpAdd;
        case 335 /*GLFW_KEY_KP_ENTER*/:     return Key::KpEnter;
        case 336 /*GLFW_KEY_KP_EQUAL*/:     return Key::KpEqual;

        // Modifiers
        case 340 /*GLFW_KEY_LEFT_SHIFT*/:   return Key::LeftShift;
        case 341 /*GLFW_KEY_LEFT_CONTROL*/: return Key::LeftControl;
        case 342 /*GLFW_KEY_LEFT_ALT*/:     return Key::LeftAlt;
        case 343 /*GLFW_KEY_LEFT_SUPER*/:   return Key::LeftSuper;
        case 344 /*GLFW_KEY_RIGHT_SHIFT*/:  return Key::RightShift;
        case 345 /*GLFW_KEY_RIGHT_CONTROL*/:return Key::RightControl;
        case 346 /*GLFW_KEY_RIGHT_ALT*/:    return Key::RightAlt;
        case 347 /*GLFW_KEY_RIGHT_SUPER*/:  return Key::RightSuper;
        case 348 /*GLFW_KEY_MENU*/:         return Key::Menu;
        
        default:                            return Key::Unknown;
        }
    }

};
