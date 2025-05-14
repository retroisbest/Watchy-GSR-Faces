// PongGame
//14/05/2025 why doesn't the ball move by itself? Stay Awake is set to true.
//Ball only moves if right upper or lower button is held down (guessing due to screen updates when buttons pressed to move paddle)

RTC_DATA_ATTR uint8_t PongStyle;
int PongState = 0;

int ballX = 70;
int ballY = 100;
int ballDirX = -1;
int ballDirY = 1;
int paddleY = 80;
int score = 0;
int highScore = 0;
unsigned long lastUpdate = 0;
int speed = 500; // Lower is faster (ms between redraws)

class PongGameClass : public WatchyGSR {
public:
  PongGameClass() : WatchyGSR() { initAddOn(this); }

  void RegisterWatchFaces() {
    PongStyle = AddWatchStyle("Pong", this, true);
  }

  void InsertInitWatchStyle(uint8_t StyleID) {
    if (StyleID == PongStyle) {
      resetGame();
    }
  }

  void InsertDrawWatchStyle(uint8_t StyleID) override {
    if (StyleID != PongStyle) return;
    if (millis() - lastUpdate < speed) return;
    lastUpdate = millis();

    display.fillScreen(GxEPD_BLACK);
    display.setTextColor(GxEPD_WHITE);

    // Draw Ball
    display.fillRect(ballX, ballY, 4, 4, GxEPD_WHITE);

    // Draw Paddle (taller)
    display.fillRect(0, paddleY, 4, 32, GxEPD_WHITE);

    // Draw Score
    display.setFont(&FreeMonoBold9pt7b);
    display.setCursor(100, 20);
    display.print("Score: ");
    display.print(score);

    display.setCursor(100, 40);
    display.print("High: ");
    display.print(highScore);

    // Game Logic
    ballX += ballDirX;
    ballY += ballDirY;

    // Bounce off top/bottom
    if (ballY <= 0 || ballY >= 200 - 4) ballDirY *= -1;

    // Paddle collision or miss
    if (ballX <= 4) {
      if (ballY + 4 >= paddleY && ballY <= paddleY + 32) {
        ballDirX *= -1;
        ballX = 4;
        score++;
        if (score > highScore) highScore = score;
        if (score % 5 == 0 && speed > 100) speed -= 25;
        Pulse(); // Bounce feedback
      } else {
        Pulse(100); // Missed paddle
        delay(100);
        Pulse(100); // Extra buzz for game over
        resetGame();
      }
    }

    // Bounce off right wall
    if (ballX >= 200 - 4) {
      ballDirX *= -1;
    }

    UpdateScreen();
  }

  void InsertOnMinute() override {
    if (!GameStatusOn() && WatchTime.Local.Minute == 30) {
      GameStatus(true);
    }
  }

  bool InsertNeedAwake(bool GoingAsleep) override {
    return true; // Keeps active mode going for continuous play
  }

  bool InsertHandlePressed(uint8_t SwitchNumber, bool &Haptic, bool &Refresh) override {
    switch (SwitchNumber) {
      case 1: // Menu
        return false;
      case 2: // Back
        GameStatus(false);
        HideGame();
        Haptic = true;
        return true;
      case 3: // Up
        if (paddleY > 0) paddleY -= 6; // Bigger step
        Refresh = true;
        return true;
      case 4: // Down
        if (paddleY < 200 - 32) paddleY += 6; // Bigger step
        Refresh = true;
        return true;
    }
    return false;
  }

  void SaveProgress() {}

private:
  void resetGame() {
    ballX = 70;
    ballY = 100;
    ballDirX = -1;
    ballDirY = 1;
    paddleY = 80;
    score = 0;
    speed = 500;
  }

  void Pulse(uint16_t duration = 50) {
    VibeTo(true);
    delay(duration);
  }
};

PongGameClass PongGameClassLoader;

