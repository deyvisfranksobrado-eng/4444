#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <iostream>

constexpr float PI2=6.28318530718f, RX=115, RZ=78;
constexpr int LAPS=3;
const Color cyan={35,235,255,255}, pink={255,44,137,255}, asphalt={19,23,38,255};
float wrap(float a){return a-std::floor(a/PI2)*PI2;}
Vector3 track(float a,float lane=0){return {(RX+lane)*std::cos(a),0,(RZ+lane)*std::sin(a)};}
Vector3 tangent(float a){return Vector3Normalize({-RX*std::sin(a),0,RZ*std::cos(a)});}
struct Driver {float angle=0,lane=0,speed=0,nitro=100; Color color=cyan; bool done=false; float finishTime=-1;};
struct Race {
    std::array<Driver,5> cars{}; float elapsed=0; int place=1;
    void reset(){elapsed=0;place=1;for(int i=0;i<5;i++)cars[i]={-0.04f*i,float(i%3-1)*3,0,100,i==0?cyan:Color{(unsigned char)(140+i*20),(unsigned char)(65+i*28),(unsigned char)(210-i*20),255},false};}
    void step(float dt,float throttle,float steer,bool boost,bool brake){
        elapsed+=dt;
        for(int i=0;i<5;i++){
            auto &c=cars[i];if(c.done)continue;
            bool nitro=i==0&&boost&&c.nitro>1&&throttle>0;
            float target=i==0?(throttle>0?(nitro?86.f:62.f):0.f):54.f+i*1.5f+3*std::sin(elapsed*0.6f+i);
            if(i==0&&brake)target=0;
            float acceleration=target>c.speed?(nitro?34.f:19.f):(brake?60.f:10.f);
            c.speed+=std::clamp(target-c.speed,-acceleration*dt,acceleration*dt);
            if(i==0){c.lane=std::clamp(c.lane+steer*dt*(3+c.speed*.09f),-10.f,10.f);c.nitro=std::clamp(c.nitro+(nitro?-32.f:9.f)*dt,0.f,100.f);if(std::fabs(c.lane)>8)c.speed=std::max(0.f,c.speed-32*dt);}
            else c.lane+=(std::sin(elapsed*.8f+i)*5-c.lane)*dt;
            Vector3 t={-RX*std::sin(c.angle),0,RZ*std::cos(c.angle)};
            float previous=c.angle;
            c.angle+=c.speed*dt/Vector3Length(t);
            if(c.angle>=PI2*LAPS){c.finishTime=elapsed-dt+dt*(PI2*LAPS-previous)/(c.angle-previous);c.angle=PI2*LAPS;c.done=true;}
        }
        for(int i=1;i<5;i++){
            auto &a=cars[0];auto &b=cars[i];float da=std::remainder(a.angle-b.angle,PI2);
            if(std::fabs(da)<.035f&&std::fabs(a.lane-b.lane)<2.4f){float sign=a.lane>=b.lane?1.f:-1.f;a.lane=std::clamp(a.lane+sign*dt*14,-10.f,10.f);a.speed=std::max(0.f,a.speed-18*dt);}
        }
        place=1;for(int i=1;i<5;i++)if(cars[0].done?(cars[i].done&&cars[i].finishTime<cars[0].finishTime):cars[i].angle>cars[0].angle)place++;
    }
};
void box(Vector3 p,Vector3 size,Color color){DrawCubeV(p,size,color);}
void car(const Driver &d,bool boosting){
    Vector3 p=track(d.angle,d.lane);Vector3 t=tangent(d.angle);
    rlPushMatrix();rlTranslatef(p.x,.5f,p.z);rlRotatef(std::atan2(t.x,t.z)*RAD2DEG,0,1,0);
    box({0,-.32f,0},{2.5f,.06f,4.9f},{3,6,12,170});
    box({0,.35f,0},{2.25f,.65f,4.4f},d.color);
    box({0,.7f,-.15f},{1.8f,.5f,2.4f},ColorBrightness(d.color,-.3f));
    box({0,1.03f,-.15f},{1.5f,.18f,1.55f},{13,24,45,255});
    box({0,.88f,.8f},{1.6f,.35f,.12f},{100,205,240,255});
    box({0,.75f,-1.96f},{2.6f,.12f,.55f},{12,14,25,255});
    for(float x:{-1.15f,1.15f})for(float z:{-1.35f,1.35f}){box({x,.05f,z},{.35f,.65f,.9f},{9,10,16,255});box({x*1.03f,.07f,z},{.04f,.32f,.55f},GRAY);}
    for(float x:{-.75f,.75f}){box({x,.42f,2.23f},{.6f,.15f,.07f},WHITE);box({x,.4f,-2.23f},{.6f,.14f,.07f},pink);if(boosting)DrawSphere({x,.1f,-2.8f},.35f,cyan);}
    box({0,.7f,1.4f},{.12f,.04f,1.2f},WHITE);
    rlPopMatrix();
}
void world(float time){
    ClearBackground({7,10,24,255});
    DrawPlane({0,-.18f,0},{1700,1700},{9,14,27,255});
    DrawSphere({-260,100,-340},38,{224,119,144,255});
    for(int i=0;i<128;i++){
        float a=i*PI2/128,b=(i+1)*PI2/128;
        Vector3 p1=track(a,-11),p2=track(a,11),p3=track(b,11),p4=track(b,-11);
        DrawTriangle3D(p1,p3,p2,asphalt);DrawTriangle3D(p1,p4,p3,asphalt);
        for(float side:{-11.f,11.f}){Vector3 u=track(a,side),v=track(b,side);u.y=v.y=.14f;DrawLine3D(u,v,side<0?pink:cyan);}
        if(i%2==0){Vector3 u=track(a),v=track(a+.025f);u.y=v.y=.02f;DrawLine3D(u,v,{115,128,159,255});}
        if(i%4==0){Vector3 p=track(a,13);box({p.x,1,p.z},{.4f,2,.4f},cyan);DrawSphere({p.x,2,p.z},.3f,WHITE);}
    }
    for(int i=0;i<90;i++){
        float a=i*2.399963f,r=180+(i%7)*24.f;
        float h=12+((i*37)%69);Vector3 p={std::cos(a)*r,h/2,std::sin(a)*r};
        box(p,{12.f+(i%4)*3,h,14},{(unsigned char)(15+i%3*4),20,(unsigned char)(36+i%5*3),255});
        for(float y=4;y<h;y+=7)box({p.x,y,p.z-7.05f},{8,.5f,.08f},i%2?Color{38,102,151,255}:Color{107,38,105,255});
    }
    for(int i=0;i<12;i++){float a=i*PI2/12;Vector3 p=track(a,-25);box({p.x,2,p.z},{1,4,1},{24,34,48,255});DrawSphere({p.x,5,p.z},4,{18,55,54,255});}
    box({RX,6,0},{24,.65f,1},pink);for(float x:{RX-12,RX+12})box({x,3,0},{.7f,6,.7f},cyan);
    for(int i=0;i<22;i++)for(int j=0;j<2;j++)box({RX-10.5f+i,.025f,float(j)-.5f},{1,.03f,1},(i+j)%2?WHITE:BLACK);
    for(int i=0;i<30;i++){float a=i*PI2/30+time*.008f;DrawSphere({std::cos(a)*480,130+float(i%6)*17,std::sin(a)*480},.6f,{130,160,210,255});}
}
void label(const char* s,int x,int y,int size,Color c=WHITE){DrawText(s,x,y,size,c);}
void center(const char* s,int y,int size,Color c=WHITE){label(s,(GetScreenWidth()-MeasureText(s,size))/2,y,size,c);}
void panel(int x,int y,int w,int h){DrawRectangle(x,y,w,h,{8,14,29,225});DrawRectangle(x,y,3,h,cyan);}
int selfTest(){
    Race r;r.reset();for(int i=0;i<60*15;i++)r.step(1.f/60,1,0,false,false);
    if(r.cars[0].angle<=PI2||r.cars[0].speed<60||r.cars[0].nitro!=100)return 1;
    float initial=r.cars[0].nitro;for(int i=0;i<60;i++)r.step(1.f/60,1,0,true,false);
    if(r.cars[0].nitro>=initial||r.cars[0].speed<=62)return 2;
    for(int i=0;i<120;i++)r.step(1.f/60,0,1,false,true);
    if(r.cars[0].speed>1||r.cars[0].lane>10)return 3;
    // Return to the asphalt after checking the off-road lane limit.
    for(int i=0;i<60*120&&!r.cars[0].done;i++)r.step(1.f/60,1,std::clamp(-r.cars[0].lane,-1.f,1.f),false,false);
    if(!r.cars[0].done||r.cars[0].angle!=PI2*LAPS)return 4;
    r.reset();if(r.elapsed!=0||r.cars[0].done||r.cars[0].angle!=0)return 5;
    // A rival already crossed the line: a tied final angle must not award first place.
    r.elapsed=10;r.cars[0].angle=PI2*LAPS-.01f;r.cars[0].speed=62;
    r.cars[1].angle=PI2*LAPS;r.cars[1].done=true;r.cars[1].finishTime=9;
    r.step(.05f,1,0,false,false);
    if(!r.cars[0].done||r.place!=2)return 6;
    std::cout<<"PASS: acceleration, lap progression, nitro, braking, lane limits, finish and restart\n";return 0;
}
int main(int argc,char** argv){
    if(argc>1&&std::strcmp(argv[1],"--self-test")==0)return selfTest();
    bool smoke=argc>1&&std::strcmp(argv[1],"--smoke")==0;
    SetConfigFlags(FLAG_MSAA_4X_HINT|FLAG_WINDOW_RESIZABLE);InitWindow(1280,720,"NEON APEX | C++ Racing");SetTargetFPS(60);SetExitKey(KEY_NULL);
    enum Mode{TITLE,INTRO,COUNTDOWN,RACING,FINISH};Mode mode=TITLE;Race race;race.reset();float clock=0,timer=0;bool paused=false;int cameraMode=0,frames=0;
    Camera3D camera={{0,20,0},{0,0,0},{0,1,0},55,CAMERA_PERSPECTIVE};
    const char* fragment=R"GLSL(#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
out vec4 finalColor;
void main(){
    vec2 uv=fragTexCoord;
    vec2 pixel=1.0/vec2(textureSize(texture0,0));
    vec3 color=texture(texture0,uv).rgb;
    vec3 glow=vec3(0.0);
    for(int x=-2;x<=2;x++) for(int y=-2;y<=2;y++){
        vec3 s=texture(texture0,uv+vec2(x,y)*pixel*2.0).rgb;
        glow+=max(s-vec3(0.55),vec3(0.0))/25.0;
    }
    color+=glow*0.8;
    float vignette=1.0-0.38*dot((uv-0.5)*1.4,(uv-0.5)*1.4);
    finalColor=vec4(color*vignette,1.0)*fragColor*colDiffuse;
})GLSL";
    Shader post=LoadShaderFromMemory(nullptr,fragment);
    RenderTexture2D scene=LoadRenderTexture(GetScreenWidth(),GetScreenHeight());
    while(!WindowShouldClose()){
        float dt=std::min(GetFrameTime(),.05f);clock+=dt;if(!paused)timer+=dt;
        if(IsKeyPressed(KEY_ESCAPE)){if(mode==RACING)paused=!paused;else break;}
        if(IsKeyPressed(KEY_ENTER)&&(mode==TITLE||mode==FINISH)){race.reset();mode=INTRO;timer=0;paused=false;}
        if(IsKeyPressed(KEY_R)&&mode!=TITLE){race.reset();mode=INTRO;timer=0;paused=false;}
        if(IsKeyPressed(KEY_C))cameraMode=(cameraMode+1)%3;
        if(mode==INTRO&&(timer>8||IsKeyPressed(KEY_SPACE))){mode=COUNTDOWN;timer=0;}
        if(mode==COUNTDOWN&&timer>=3){mode=RACING;timer=0;}
        bool boost=IsKeyDown(KEY_LEFT_SHIFT)||IsKeyDown(KEY_RIGHT_SHIFT);
        if(smoke&&frames==30){mode=INTRO;timer=0;}
        if(smoke&&frames==90){mode=COUNTDOWN;timer=0;}
        if(smoke&&frames==120){mode=RACING;timer=0;}
        if(mode==RACING&&!paused){float throttle=smoke||IsKeyDown(KEY_W)||IsKeyDown(KEY_UP)?1.f:0.f;float steer=float(IsKeyDown(KEY_D)||IsKeyDown(KEY_RIGHT))-float(IsKeyDown(KEY_A)||IsKeyDown(KEY_LEFT));race.step(dt,throttle,steer,boost,IsKeyDown(KEY_S)||IsKeyDown(KEY_DOWN));if(race.cars[0].done){mode=FINISH;timer=0;}}
        auto &player=race.cars[0];Vector3 p=track(player.angle,player.lane),t=tangent(player.angle);Vector3 desired;
        if(mode==TITLE){camera.position={std::cos(clock*.13f)*170,75,std::sin(clock*.13f)*130};camera.target={0,0,0};}
        else if(mode==INTRO||mode==FINISH){float a=(mode==INTRO?timer*.75f:timer*.35f);camera.position=Vector3Add(p,{std::cos(a)*12,3+std::sin(a*.5f)*2,std::sin(a)*12});camera.target=Vector3Add(p,{0,1,0});if(mode==INTRO&&timer<2.6f){camera.position={170,65,90-timer*12};camera.target={40,0,0};}else if(mode==INTRO&&timer<5.3f){camera.position=Vector3Add(p,{4,1.8f,6-timer*.5f});}}
        else {float distance=cameraMode==0?10:cameraMode==1?3:20;desired=Vector3Add(Vector3Subtract(p,Vector3Scale(t,distance)),{0,cameraMode==0?5.f:cameraMode==1?2.4f:12.f,0});camera.position=Vector3Lerp(camera.position,desired,1-std::exp(-dt*6));camera.target=Vector3Add(Vector3Add(p,Vector3Scale(t,8)),{0,1,0});}
        camera.fovy=55+(mode==RACING?player.speed*.16f:0);
        if(IsWindowResized()){UnloadRenderTexture(scene);scene=LoadRenderTexture(GetScreenWidth(),GetScreenHeight());}
        BeginTextureMode(scene);BeginMode3D(camera);world(clock);for(int i=0;i<5;i++)car(race.cars[i],i==0&&boost&&mode==RACING&&player.nitro>1&&player.speed>62);EndMode3D();EndTextureMode();
        BeginDrawing();ClearBackground(BLACK);BeginShaderMode(post);DrawTextureRec(scene.texture,{0,0,float(scene.texture.width),-float(scene.texture.height)},{0,0},WHITE);EndShaderMode();
        int w=GetScreenWidth(),h=GetScreenHeight();
        if(mode==TITLE){DrawRectangle(0,0,w,h,{3,8,22,100});label("N / A   MOTORSPORT",55,45,20,cyan);label("NEON",55,h/2-80,92);label("APEX",55,h/2+5,92,cyan);label("MIDNIGHT CIRCUIT  /  THREE LAPS  /  FIVE DRIVERS",60,h/2+115,18);panel(55,h-115,420,64);label("ENTER  -  INICIAR CARRERA",75,h-94,24);label("C++  /  PROCEDURAL 3D",w-300,h-40,18,GRAY);}
        else if(mode==INTRO||mode==FINISH){DrawRectangle(0,0,w,65,BLACK);DrawRectangle(0,h-80,w,80,BLACK);label("NEON APEX   /   MIDNIGHT RUN",30,23,20,cyan);if(mode==INTRO){center(timer<3?"EL CIRCUITO DESPIERTA":timer<6?"CINCO PILOTOS. UNA META.":"TU NOCHE EMPIEZA AHORA.",h-57,24);label("ESPACIO: saltar",w-210,25,18,GRAY);}else{center(TextFormat("CARRERA COMPLETADA   /   POSICION %d",race.place),100,34,cyan);center(TextFormat("TIEMPO  %.2f s",race.elapsed),147,24);center("ENTER: volver a correr   |   ESC: salir",h-55,22);}}
        else{
            panel(25,25,220,103);label(TextFormat("POSICION  %d / 5",race.place),42,42,26);label(TextFormat("VUELTA    %d / %d",std::min(LAPS,int(player.angle/PI2)+1),LAPS),42,82,23,cyan);
            panel(w-235,25,210,70);label(TextFormat("%02d:%05.2f",int(race.elapsed)/60,std::fmod(race.elapsed,60.f)),w-216,45,32);
            panel(w-285,h-155,260,125);label(TextFormat("%03d",int(player.speed*3.6f)),w-266,h-146,58);label("KM/H",w-144,h-113,20,GRAY);DrawRectangle(w-263,h-65,215,12,{37,45,60,255});DrawRectangle(w-263,h-65,int(player.nitro*2.15f),12,cyan);label("SHIFT / NITRO",w-263,h-45,13,cyan);
            panel(25,h-90,520,60);label("W / ACELERAR   S / FRENAR   A D / GIRAR",40,h-78,18);label("C / CAMARA   R / REINICIAR   ESC / PAUSA",40,h-54,16,GRAY);
            // Mini-map uses the same elliptical path and driver coordinates.
            panel(25,145,180,145);for(int i=0;i<64;i++){float a=i*PI2/64,b=(i+1)*PI2/64;DrawLineEx({115+70*std::cos(a),217+48*std::sin(a)},{115+70*std::cos(b),217+48*std::sin(b)},4,{60,75,99,255});}for(int i=4;i>=0;i--)DrawCircleV({115+70*std::cos(race.cars[i].angle),217+48*std::sin(race.cars[i].angle)},i==0?5:3,race.cars[i].color);
            if(mode==COUNTDOWN)center(TextFormat("%d",3-int(timer)),h/2-60,100,cyan);
            if(mode==RACING&&race.elapsed<1)center("GO!",h/2-50,90,cyan);
            if(paused){DrawRectangle(0,0,w,h,{0,0,0,170});center("PAUSA",h/2-45,60);center("ESC para continuar",h/2+30,22,cyan);}
        }
        EndDrawing();if(smoke&&++frames>=240){TakeScreenshot("/tmp/neon-apex-smoke.png");break;}
    }
    UnloadRenderTexture(scene);UnloadShader(post);CloseWindow();return 0;
}
