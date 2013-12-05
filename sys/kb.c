#include <sys/kb.h>
#include <sys/idt.h>
#include<stdio.h>

bool _shift, _ctrl, _caps, _alt;
extern void irq_install_handler(int, void*);
char keycode_to_ascii(uint8_t);
extern int reading;
extern char* io_buff;

static int scan_code[] = {

  //! key     scancode
  KEY_UNKNOWN,  //0
  KEY_ESCAPE,   //1
  KEY_1,      //2
  KEY_2,      //3
  KEY_3,      //4
  KEY_4,      //5
  KEY_5,      //6
  KEY_6,      //7
  KEY_7,      //8
  KEY_8,      //9
  KEY_9,      //0xa
  KEY_0,      //0xb
  KEY_MINUS,    //0xc
  KEY_EQUAL,    //0xd
  KEY_BACKSPACE,  //0xe
  KEY_TAB,    //0xf
  KEY_Q,      //0x10
  KEY_W,      //0x11
  KEY_E,      //0x12
  KEY_R,      //0x13
  KEY_T,      //0x14
  KEY_Y,      //0x15
  KEY_U,      //0x16
  KEY_I,      //0x17
  KEY_O,      //0x18
  KEY_P,      //0x19
  KEY_LEFTBRACKET,//0x1a
  KEY_RIGHTBRACKET,//0x1b
  KEY_RETURN,   //0x1c
  KEY_LCTRL,    //0x1d
  KEY_A,      //0x1e
  KEY_S,      //0x1f
  KEY_D,      //0x20
  KEY_F,      //0x21
  KEY_G,      //0x22
  KEY_H,      //0x23
  KEY_J,      //0x24
  KEY_K,      //0x25
  KEY_L,      //0x26
  KEY_SEMICOLON,  //0x27
  KEY_QUOTE,    //0x28
  KEY_GRAVE,    //0x29
  KEY_LSHIFT,   //0x2a
  KEY_BACKSLASH,  //0x2b
  KEY_Z,      //0x2c
  KEY_X,      //0x2d
  KEY_C,      //0x2e
  KEY_V,      //0x2f
  KEY_B,      //0x30
  KEY_N,      //0x31
  KEY_M,      //0x32
  KEY_COMMA,    //0x33
  KEY_DOT,    //0x34
  KEY_SLASH,    //0x35
  KEY_RSHIFT,   //0x36
  KEY_KP_ASTERISK,//0x37
  KEY_RALT,   //0x38
  KEY_SPACE,    //0x39
  KEY_CAPSLOCK, //0x3a
  KEY_F1,     //0x3b
  KEY_F2,     //0x3c
  KEY_F3,     //0x3d
  KEY_F4,     //0x3e
  KEY_F5,     //0x3f
  KEY_F6,     //0x40
  KEY_F7,     //0x41
  KEY_F8,     //0x42
  KEY_F9,     //0x43
  KEY_F10,    //0x44
  KEY_KP_NUMLOCK, //0x45
  KEY_SCROLLLOCK, //0x46
  KEY_HOME,   //0x47
  KEY_KP_8,   //0x48  //keypad up arrow
  KEY_PAGEUP,   //0x49
  KEY_KP_2,   //0x50  //keypad down arrow
  KEY_KP_3,   //0x51  //keypad page down
  KEY_KP_0,   //0x52  //keypad insert key
  KEY_KP_DECIMAL, //0x53  //keypad delete key
  KEY_UNKNOWN,  //0x54
  KEY_UNKNOWN,  //0x55
  KEY_UNKNOWN,  //0x56
  KEY_F11,    //0x57
  KEY_F12     //0x58
};

//! read status from keyboard controller
uint8_t kybrd_ctrl_read_status() { 
  return inb (KYBRD_CTRL_STATS_REG);
}

/*
 * To send a command we need to ensure if the keyboard controller is ready for it.
 * We can check this by seeing if the input buffer is full or not. If the bit is 0
 * we can send the command. 
 */
void kybrd_ctrl_send_cmd(uint8_t cmd) {
  //! wait for kkybrd controller input buffer to be clear
  while (1){
      if ( (kybrd_ctrl_read_status() & KYBRD_CTRL_STATS_MASK_IN_BUF) == 0)
          break;
  }      
  outb (KYBRD_CTRL_CMD_REG, cmd);
}

//! read keyboard encoder buffer
uint8_t keybrd_enc_read_buf() {
  return inb (KYBRD_ENC_INPUT_BUF);
}

//! send command byte to keyboard encoder
void kybrd_enc_send_cmd (uint8_t cmd) {
//! wait for kkybrd controller input buffer to be clear
  while (1){
    if ( (kybrd_ctrl_read_status() & KYBRD_CTRL_STATS_MASK_IN_BUF) == 0)
      break;
  }
  outb (KYBRD_ENC_CMD_REG, cmd);
}


void keyboard_handler(regs *r){
  uint8_t scancode;
  KEYCODE key;
  
  // Read the scan code only if the keyboard controller output buffer is full.
  if (kybrd_ctrl_read_status() & KYBRD_CTRL_STATS_MASK_OUT_BUF) {
      //read the code
      scancode = keybrd_enc_read_buf();
      key = scan_code[scancode];
      //Check if CAPS or SHIFT or any special key is pressed
      if(scancode == 0xE0 || scancode == 0xE1){}
      else{
          // See if this is break code. i.e Test bit 7 of the received scan code
          if(scancode & 0x80){
              // convert the break code into its make code equivalent. The make code
              // and break codes are separated by 0x80
              scancode -= 0x80;
              key = scan_code[scancode];
              switch(key){
                  case KEY_LCTRL:
                  case KEY_RCTRL:
                          _ctrl = FALSE;
                          break;

                  case KEY_LSHIFT:
                  case KEY_RSHIFT:
                          _shift = FALSE;
                          break;

                  case KEY_LALT:
                  case KEY_RALT:
                          _alt = FALSE;
                          break;
                  default:
                          break;
              }
          }
          else{
              //This is a make code
              
              key = scan_code[scancode];

              switch(key){
                  case KEY_LCTRL:
                  case KEY_RCTRL:
                          _ctrl = TRUE;
                          break;

                  case KEY_LSHIFT:
                  case KEY_RSHIFT:
                          _shift = TRUE;
                          break;

                  case KEY_LALT:
                  case KEY_RALT:
                          _alt = TRUE;
                          break;

                  // We handle only the make code for CAPSLOCK.
                  // Break for CAPSLOCK does not matter
                  case KEY_CAPSLOCK:
                          _caps = (_caps) ? FALSE : TRUE;
                          break;
                  default:
                            write_char(TEXT_COLOR,keycode_to_ascii(key));    
                          if(reading){
                            //printf("calling ip buff\n");
                            write_io_buff(keycode_to_ascii(key));
                            }
                          break;
              }

          }
      }
  }
}

char keycode_to_ascii(uint8_t code){
    KEYCODE key = code;

    if(_shift || _caps){
        if(key >= 'a' && key <= 'z')
            key -= 32;
        else if(key >= '0' && key <= '9'){
            switch(key){
                case '0':
                      key = KEY_RIGHTPARENTHESIS;
                      break;
                case '1':
                      key = KEY_EXCLAMATION;
                      break;
                case '2':
                      key = KEY_AT;
                      break;
                case '3':
                      key = KEY_HASH;
                      break;
                case '4':
                      key = KEY_DOLLAR;
                      break;
                case '5':
                      key = KEY_PERCENT;
                      break;
                case '6':
                      key = KEY_CARRET;
                      break;
                case '7':
                      key = KEY_AMPERSAND;
                      break;
                case '8':
                      key = KEY_ASTERISK;
                      break;
                case '9':
                      key = KEY_LEFTPARENTHESIS;
                      break;
                default:
                      break;
            }
        }
        else{
            switch(key){
                case KEY_COMMA:
                      key = KEY_LESS;
                      break;
                case KEY_DOT:
                      key = KEY_GREATER;
                      break;
                case KEY_SLASH:
                      key = KEY_QUESTION;
                      break;
                case KEY_SEMICOLON:
                      key = KEY_COLON;
                      break;
                case KEY_QUOTE:
                      key = KEY_QUOTEDOUBLE;
                      break;
                case KEY_LEFTBRACKET:
                      key = KEY_LEFTCURL;
                      break;
                case KEY_RIGHTBRACKET:
                      key = KEY_RIGHTCURL;
                      break;
                case KEY_MINUS:
                      key = KEY_UNDERSCORE;
                      break;
                case KEY_PLUS:
                      key = KEY_EQUAL;
                      break;
                case KEY_BACKSLASH:
                      key = KEY_BAR;
                      break;
                case KEY_GRAVE:
                      key = KEY_TILDE;
                      break;
                default:
                      break;  
            }
        }
    }
    return key;
}

void keyboard_install(){
    irq_install_handler(1, keyboard_handler);
}
