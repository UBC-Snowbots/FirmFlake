//MCU for general left panel

#define BAUDRATE 9600
#define TX_RATE_HZ 30


#define SW_T00_PIN 2
//T0 is just one flat switch currently

#define SW_T10_PIN 3
#define SW_T11_PIN 4

#define SW_T20_PIN 5
#define SW_T21_PIN 6

#define BT_T30_PIN 7
#define BT_T31_PIN 8

#define BT_T40_PIN 9
#define BT_T41_PIN 10

#define BT_T50_PIN 11 //This Button Needs replacement
#define BT_T51_PIN 12


long interval_ms = 99;
long last_time = 0;
long current_time = 0;

void setup() {
  pinMode(SW_T00_PIN, INPUT_PULLUP);
  pinMode(SW_T10_PIN, INPUT_PULLUP);
  pinMode(SW_T11_PIN, INPUT_PULLUP);
  pinMode(SW_T20_PIN, INPUT_PULLUP);
  pinMode(SW_T21_PIN, INPUT_PULLUP);
  pinMode(BT_T30_PIN, INPUT_PULLUP);
  pinMode(BT_T31_PIN, INPUT_PULLUP);
  pinMode(BT_T40_PIN, INPUT_PULLUP);
  pinMode(BT_T41_PIN, INPUT_PULLUP);
  pinMode(BT_T50_PIN, INPUT_PULLUP);
  pinMode(BT_T51_PIN, INPUT_PULLUP);
  interval_ms = 1000/TX_RATE_HZ;
  
  Serial.begin(BAUDRATE);

}

void loop() {
current_time = millis();
  if(current_time - last_time > interval_ms){
      last_time = millis();
      //might wanna use arrays but ehh our main problem is gonna be not mixing up the inputs
  int SW_T00 = digitalRead(SW_T00_PIN) == LOW ? 1 : 0;
  int SW_T10 = digitalRead(SW_T10_PIN) == LOW ? 1 : 0;
  int SW_T11 = digitalRead(SW_T11_PIN) == LOW ? 0 : 1;
  int SW_T20 = digitalRead(SW_T20_PIN) == LOW ? 1 : 0;
  int SW_T21 = digitalRead(SW_T21_PIN) == LOW ? 1 : 0;;
  int BT_T30 = digitalRead(BT_T30_PIN);
  int BT_T31 = digitalRead(BT_T31_PIN);
  int BT_T40 = digitalRead(BT_T40_PIN);
  int BT_T41 = digitalRead(BT_T41_PIN);
  int BT_T50 = digitalRead(BT_T50_PIN);
  int BT_T51 = digitalRead(BT_T51_PIN);


//  int SW_T30 = digitalRead(SW_T00_PIN);

  String outmsg = "$gen_left_A(";
  outmsg = outmsg + SW_T00 + ","; // for some reason += messes up the string
  outmsg = outmsg + SW_T10 + ",";
  outmsg = outmsg + SW_T11 + ",";
  outmsg = outmsg + SW_T20 + ",";
  outmsg = outmsg + SW_T21 + ",";
  outmsg = outmsg + BT_T30 + ",";
  outmsg = outmsg + BT_T31 + ",";
  outmsg = outmsg + BT_T40 + ",";
  outmsg = outmsg + BT_T41 + ",";
  outmsg = outmsg + BT_T50 + ",";
  outmsg = outmsg + BT_T51;

//  outmsg += SW_T10 + "";
  outmsg = outmsg + ")";
  Serial.println(outmsg);

  }
  delay(1);
}
