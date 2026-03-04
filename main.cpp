#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>

using namespace std;

Color blue = {0, 121, 241, 255};
Color red = {230, 41, 55, 255};
Color green = {0, 228, 48, 255};
Color darkgreen = {0, 100, 0, 255};
Color black = {0, 0, 0, 255};
Color brickred = {178, 34, 34, 255};

int cellSize = 30;
int cellCount = 25;
int offset = 30;

double lastUpdateTime = 0;

bool eventTriggered(double interval){
    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval){
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

bool ElementInDeque(Vector2 element, deque <Vector2> deque){
    for (int i = 0; i < deque.size(); i++){
        if (Vector2Equals(deque[i], element)){
            return true;
        }
    }
    return false;
}

bool ElementInElement(Vector2 element1, Vector2 element2){
    if (Vector2Equals(element1, element2)){
        return true;
    }
    return false;
}

bool ElementInFrontOfHead(Vector2 element){
    for (float i = 3; i < 7; i = i + 1){
        for (float j = 22; j < 25; j = j + 1){
            Vector2 position = {i, j};
            if (element == position){
                return true;
            }
        }
    }
    return false;
}

class Stone
{
public:
    Vector2 position;
    Texture2D stonetexture;

    Stone (Vector2 foodpos, deque <Vector2> snakeBody, deque <Stone> otherStones, Texture2D tex){
        stonetexture = tex;
        position = GenerateRandomPos(foodpos, snakeBody, otherStones);
    }

    ~Stone (){

    }

    void Draw()
    {
        DrawTexture(stonetexture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
    }

    Vector2 GenerateRandomPos(Vector2 foodpos, deque <Vector2> snakeBody, deque <Stone> otherStones){
        Vector2 pos = GenerateRandomCell();
        while (ElementInElement(pos, foodpos) || ElementInDeque(pos, snakeBody) || ElementInFrontOfHead(pos) || StoneOverlaps(pos, otherStones)){
            pos = GenerateRandomCell();
        }
        return pos;
    }

    Vector2 GenerateRandomCell(){
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{x, y};
    }

    bool StoneOverlaps(Vector2 pos, deque <Stone> otherStones){
        for (auto &s : otherStones){
            if (ElementInElement(pos, s.position)){
                return true;
            }
        }
        return false;
    }
};

bool ElementInStones(Vector2 element, deque <Stone> stones){
    for (auto &stone : stones){
        if (Vector2Equals(stone.position, element)){
            return true;
        }
    }
    return false;
}

class Snake
{
public:
    deque <Vector2> body = {Vector2{2, 24}, Vector2{1, 24}, Vector2{0, 24}};
    Vector2 direction = {1, 0};
    bool addSegment = false;
    Texture2D eyeTexture;

    Snake(){
        Image eyeimage = LoadImage("eyes.png");
        ImageResize(&eyeimage, cellSize, cellSize);
        eyeTexture = LoadTextureFromImage(eyeimage);
        UnloadImage(eyeimage);
    }

    ~Snake(){
        UnloadTexture(eyeTexture);
    }

    void Draw()
    {
        for (int i = 0; i < body.size(); i++){
            int x = body[i].x;
            int y = body[i].y;

            Rectangle segment = Rectangle{offset + x * cellSize, offset + y * cellSize, cellSize, cellSize};
            
            if (i == 0){
                DrawRectangleRounded(segment, 0.5, 10, darkgreen);

                float rotation = 0;
                if (direction.x == 1) rotation = 0;
                if (direction.x == -1) rotation = 180;
                if (direction.y == -1) rotation = 270;
                if (direction.y == 1) rotation = 90;

                Rectangle sourceRec = {0.0f, 0.0f, (float)eyeTexture.width, (float)eyeTexture.height};
                Rectangle destinationRec = {offset + x * cellSize + ((float)cellSize / 2), offset + y * cellSize + ((float)cellSize / 2), (float)cellSize, (float)cellSize};
                Vector2 origin = {(float)cellSize / 2, (float)cellSize / 2};

                DrawTexturePro(eyeTexture, sourceRec, destinationRec, origin, rotation, WHITE);
            }
            else {
                DrawRectangleRounded(segment, 0.5, 10, darkgreen);
            }
        }
    }

    void Update()
    {
        body.push_front(Vector2Add(body[0], direction));
        if (addSegment == true){
            addSegment = false;
        }else{
            body.pop_back();
        }
    }

    void reset(){
        body = {Vector2{2, 24}, Vector2{1, 24}, Vector2{0, 24}};
        direction = {1, 0};
    }

};

class Food
{
public:
    Vector2 position;
    Texture2D  texture;

    Food (deque <Vector2> snakeBody){
        Image image = LoadImage("food.png");
        ImageResize(&image, cellSize, cellSize);
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
        deque <Stone> emptyStones;
        position = GenerateRandomPos(snakeBody, emptyStones);
    }

    ~Food(){
        UnloadTexture(texture);
    }

    void Draw()
    {
        DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
    }

    Vector2 GenerateRandomCell(){
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{x, y};
    }

    Vector2 GenerateRandomPos(deque <Vector2> snakeBody, deque <Stone> stones){
        Vector2 position = GenerateRandomCell();
        while (ElementInDeque(position, snakeBody) || ElementInStones(position, stones)){
            position = GenerateRandomCell();
        }
        return position;
    }
};

class Game
{
public:
    Snake snake = Snake();    
    Food food = Food(snake.body);
    deque <Stone> stones;
    Texture2D stoneTexture;
    bool running = true;
    int score = 0;
    Sound eatsound;
    Sound wallsound;
    Sound wowsound;

    Game() : snake(), food(snake.body){
        InitAudioDevice();
        if (!IsAudioDeviceReady()){
            cout << "Audio device not ready!" << endl;
        }
        eatsound = LoadSound("eat.mp3");
        wallsound = LoadSound("crack.mp3");
        wowsound = LoadSound("wow.mp3");

        Image stoneimage = LoadImage("stone.png");
        ImageResize(&stoneimage, cellSize, cellSize);
        stoneTexture = LoadTextureFromImage(stoneimage);
        UnloadImage(stoneimage);

        for (int i = 0; i < 6; i++){
            stones.push_back(Stone(food.position, snake.body, stones, stoneTexture));
        }
        
    }

    ~Game(){
        UnloadSound(eatsound);
        UnloadSound(wallsound);
        UnloadSound(wowsound);
        UnloadTexture(stoneTexture);
        CloseAudioDevice();
    }

    void Draw(){
        snake.Draw();
        food.Draw();
        for (auto &stone : stones){
            stone.Draw();
        }
    }

    void Update(){
        if (running){
            snake.Update();
            checkCollisionWithFood();
            checkCollisionWithEdges();
            checkCollisionWithTail();
            checkCollisionWithStones();
        }
    }

    void checkCollisionWithFood(){
        if (Vector2Equals(snake.body[0], food.position)){
            food.position = food.GenerateRandomPos(snake.body, stones);
            snake.addSegment = true;
            score++;

            if (score > 0 && score % 10 == 0){
                StopSound(eatsound);
                PlaySound(wowsound);
            }
            else{
                PlaySound(eatsound);
            }
        }
    }

    void checkCollisionWithEdges(){
        if (snake.body[0].x == cellCount || snake.body[0].x == -1){
            GameOver();
        }
        if (snake.body[0].y == cellCount || snake.body[0].y == -1){
            GameOver();
        }
    }

    void GameOver(){
        snake.reset();
        food.position = food.GenerateRandomPos(snake.body, stones);
        running = false;
        score = 0;
        StopSound(eatsound);
        PlaySound(wallsound);
    }

    void checkCollisionWithTail(){
        deque <Vector2> headlessbody = snake.body;
        headlessbody.pop_front();
        if (ElementInDeque(snake.body[0], headlessbody)){
            GameOver();
        }
    }

    void checkCollisionWithStones(){
        for (auto &stone : stones){
            if (ElementInElement(snake.body[0], stone.position)){
                GameOver();
            }
        }
    }
};


int main()
{
    cout << "Starting The Game...." << endl;
    InitWindow(2 * offset + cellSize * cellCount, 2 * offset + cellSize * cellCount, "Snake Game");
    SetTargetFPS(60);

    Game game = Game();

    int gameState = 0;
    int screenWidth = 2 * offset + cellSize * cellCount;
    int screenHeight = 2 * offset + cellSize * cellCount;

    while (WindowShouldClose() == false)
    {
        BeginDrawing();
        ClearBackground(green);
        DrawRectangleLinesEx(Rectangle{(float)offset - 5, (float)offset - 5, (float)cellSize * cellCount + 10, (float)cellSize * cellCount + 10}, 5, brickred);
        
        if (gameState == 0){
            int titleWidth = MeasureText("SNAKE GAME", 60);
            int subtitleWidth = MeasureText("Press SPACE to Start Game", 30);
            
            DrawText("SNAKE GAME", (screenWidth - titleWidth) / 2, screenHeight / 2 - 60, 60, darkgreen);
            DrawText("Press SPACE to Start Game", (screenWidth - subtitleWidth) / 2, screenHeight / 2 + 20, 30, darkgreen);

            if (IsKeyPressed(KEY_SPACE)){
                gameState = 1;
            }
        }
        else if (gameState == 1){
            if (eventTriggered(0.2)){
                game.Update();
            }
            if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1){
                game.snake.direction = {0, -1};
                game.running = true;
            }
            if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1){
                game.snake.direction = {0, 1};
                game.running = true;
            }
            if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1){
                game.snake.direction = {-1, 0};
                game.running = true;
            }
            if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1){
                game.snake.direction = {1, 0};
                game.running = true;
            }

            DrawText("**Snake Game**", 25, 5, 20, darkgreen);
            DrawText("Score = ", offset + cellSize * cellCount - 100, 5, 20, darkgreen);
            DrawText(TextFormat("%i", game.score), offset + cellSize * cellCount - 12, 5, 20, darkgreen);
            game.Draw();
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
