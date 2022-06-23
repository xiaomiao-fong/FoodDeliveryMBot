#include <Arduino.h>
#include <Wire.h>
#include <SoftwareSerial.h>
#include <MeMCore.h>
#include <ArduinoSTL.h>
#include <vector>
#include <queue>
double angle_rad = PI/180.0;
double angle_deg = 180.0/PI;
MBotDCMotor motor(0);          // 宣告馬達物件
MeDCMotor motor_9(9);
MeDCMotor motor_10(10);
MeLineFollower linefollower_2(2);  // 宣告巡線感應物件

using namespace std;


void right();
void left();
void turnaround();
void throughthewhite();

class Node{
  public:
    char num;
    int x;
    int y;
    int parent;
    char sl; //seg low
    char sh; //seg high
    

    Node(char n, int _x, int _y, int p, char _sl, char _sh){
    
      num = n;
      x = _x;
      y = _y;
      parent = p;
      sl = _sl;
      sh = _sh;
    
    
    }
};

String findPath(char s, char e, Node * Road);
void p2c(String path, int &orix, int &oriy, queue<char> &cmd, Node * Road);
int findOri(Node a, Node b, int &Ox, int &Oy);

bool gap[11][11] = {

    { 0,0,1,0,0,0,0,0,0,0,0 },
    { 0,0,0,0,0,0,0,0,0,0,0 },
    { 1,0,0,1,0,0,0,0,0,0,0 },
    { 0,0,1,0,0,0,0,0,0,0,0 },
    { 0,0,0,0,0,0,0,0,0,0,0 },
    { 0,0,0,0,0,0,0,1,0,0,0 },
    { 0,0,0,0,0,0,0,0,0,0,0 },
    { 0,0,0,0,0,1,0,0,0,1,1 },
    { 0,0,0,0,0,0,0,0,0,0,0 },
    { 0,0,0,0,0,0,0,1,0,0,0 },
    { 0,0,0,0,0,0,0,1,0,0,0 },

};







  
  Node Road[11]={
    Node('0',0,0,2,'0','0'),
    Node('1',1,1,2,'1','1'),
    Node('2',0,1,3,'0','2'),
    Node('3',0,2,4,'0','3'),
    Node('4',3,2,-1,'0','a'),
    Node('5',3,1,4,'5','a'),
    Node('6',2,1,5,'6','6'),
    Node('7',4,1,5,'7','a'),
    Node('8',4,0,7,'8','8'),
    Node('9',5,1,7,'9','9'),
    Node('a',4,2,7,'a','a')
  };
  
  int orix = 0;
  int oriy = 1;

  
  char initp='0',shop='a',arrive='8';

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {

  queue<char> commands;
  
  
  String path = findPath(initp,shop,Road);
  //cout << path << endl;
  p2c(path, orix, oriy, commands, Road);
  
  while(!commands.empty()){
    
    Serial.println(commands.front());
    movecar(commands.front());  
    commands.pop();
    
  }


  
  path = findPath(shop,arrive,Road);
  p2c(path, orix, oriy, commands, Road);
  
  while(!commands.empty()){
    
    Serial.println(commands.front());
    movecar(commands.front());  
    commands.pop();
    
  }
  initp='0',shop='0',arrive='0';
  return 0;
  exit;
}

String findPath(char s, char e, Node * Road){
  
  Node current = Road[s < 58 ? s-'0' : s- 87];
  String fpath = "";
  fpath+=s;
  while(current.sl > e || current.sh < e){
    
    current = Road[current.parent];
    fpath += current.num;
    
  }
  
  current = Road[e < 58 ? e-'0' : e- 'W'];
  String spath = "";
  while(current.sl > s || current.sh < s){
    
    spath = current.num + spath;
    current = Road[current.parent];
    
  }
  
  
  return fpath+spath;
  
}

void p2c(String path, int &orix, int &oriy, queue<char> &cmd, Node * Road){
  
  if (path.length() == 1) return;
  vector<char> tv;
  char last = '0';
  int i = 0;
  
  /*
  
    l : left
    r : right
    s : 180
    
    w : walk
    g : one gap
    t : two gap
  
  */
  
  while(i < path.length()-1){
    
    char numa = path[i];
    char numb = path[i+1];
    
    Node a = Road[numa < 58 ? numa-'0' : numa- 'W'];
    Node b = Road[numb < 58 ? numb-'0' : numb- 'W'];
    int angle = findOri(a,b,orix,oriy);
    
    switch(angle){
      
      case 90:
        tv.push_back('l');
        last = 'l';
        break;
      
      case -90:
        tv.push_back('r');
        last = 'r';
        break;
      
      case 180:
        tv.push_back('s');
        last = 's';
        break;
        
      default:
        break;
      
    }
    
    if(gap[a.num < 58 ? a.num-'0' : a.num- 'W'][b.num < 58 ? b.num-'0' : b.num- 'W']){
      
      if(last == 'g') tv[tv.size()-1] = 't';
      else if(last == 'w') tv[tv.size()-1] = 'g';
      else tv.push_back('g');
      
    }else{
      
      if(last != 'w') tv.push_back('w');
      
    }
    
    last = tv[tv.size()-1];
    i++;
    
  }
  
  for(int j = 0; j < tv.size();j++){
    
    cmd.push(tv[j]);
    
  }
  
  return;
  
  
}

int findOri(Node a, Node b, int &Ox, int &Oy){
  
  int tOx = b.x - a.x;
  int tOy = b.y - a.y;
  int dot = Ox*tOx + Oy*tOy;
  int cross = Ox*tOy - Oy*tOx;
  
  Ox = tOx;
  Oy = tOy;
  
  if(dot != 0){
    
    if(dot > 0) return 0;
    return 180;
    
  }
  
  if(cross < 0) return -90;
  return 90;
  
}

void movecar(char carstate){
  if(carstate=='w'){
    roadtracker(0); 
  }
  if(carstate=='l'){
    left();
  }
  if(carstate=='r'){
    right();
  }
  if(carstate=='s'){
    turnaround();
  }
  if(carstate=='g'){
    roadtracker(1);
  }
  if(carstate=='t'){
    roadtracker(2);
  }
}


void roadtracker(int gaps){
  byte ReturnValue;

  while(ReturnValue != 3||gaps>0){
    
      ReturnValue = linefollower_2.readSensors();
      
      if (ReturnValue == 0){       // 兩邊都偵測到黑線
        motor.move(1,100);
      }
      if (ReturnValue == 1)      // 左邊偵測到黑線，右邊偵測到白線
      {
        motor.move(3,75);     // 左轉 速度75     
      }
      if (ReturnValue == 2)     // 左邊偵測到白線，右邊偵測到黑線
      {
        motor.move(4,75);     // 右轉 速度75     
      }
      if (ReturnValue == 3)    
      {
        if(gaps!=0){
          throughthewhite();
          gaps--;
          roadtracker(gaps);
          break;          
        } 
      motor.move(1,0);
      }         
      
  }
  
  motor.move(1,0);   
  
}   

void right(){
  motor_9.run((9) == M1 ? -100 : 100);
  motor_10.run((10) == M1 ? 100 : -100);
  delay((90+4.2)*1000/119);
  motor_9.run((9) == M1 ? 0 : 0);
  motor_10.run((10) == M1 ? 0 : 0);
}

void left(){
  motor_10.run((10) == M1 ? -100 : 100);
  motor_9.run((9) == M1 ? 100 : -100);
  delay((90+4.2)*1000/119);
  motor_10.run((9) == M1 ? 0 : 0);
  motor_9.run((10) == M1 ? 0 : 0);
}

void turnaround(){
  motor_9.run((9) == M1 ? -100 : 100);
  motor_10.run((10) == M1 ? 100 : -100);
  delay((180+4.2)*1000/119);
  motor_9.run((9) == M1 ? 0 : 0);
  motor_10.run((10) == M1 ? 0 : 0);
}

void throughthewhite(){
  motor_10.run((10) == M1 ? -100 : 100);
  motor_9.run((9) == M1 ? -100 : 100);
  delay(500);
  motor_10.run((10) == M1 ? 0 : 0);
  motor_9.run((9) == M1 ? 0 : 0);
  
}
