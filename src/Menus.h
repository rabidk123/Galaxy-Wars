#pragma once

#include <SFML/Graphics.hpp>
#include <fstream>
#include <string>
#include <iostream>

using namespace std;
using namespace sf;

struct GameState {
    int level;
    int score;
    int lives;
};

void saveGameBinary(const string& filename, int currentLevel, int score, const LIVES& lives);
bool loadGameBinary(const string& filename, int& currentLevel, int& score, LIVES& lives);



class HighScores {
private:
    string name;
    int score;
    int highScore;

    friend void debugHighScore(const HighScores& h);

    void include50Scores() {
        string names[100];
        int scores[100];
        int levels[100];
        int count = 0;

        ifstream file("TopScores.txt");
        if (file.is_open()) {
            while (count < 100 && file >> names[count] >> scores[count] >> levels[count]) {
                count++;
            }
            file.close();
        }
        //bubble sorting for descending order 
        for (int i = 0; i < count - 1; i++) {
            for (int j = 0; j < count - i - 1; j++) {
                if (scores[j] < scores[j + 1]) {
                    int tempScore = scores[j];
                    scores[j] = scores[j + 1];
                    scores[j + 1] = tempScore;

                    string tempName = names[j];
                    names[j] = names[j + 1];
                    names[j + 1] = tempName;

                    int tempLevel = levels[j];
                    levels[j] = levels[j + 1];
                    levels[j + 1] = tempLevel;
                }
            }
        }
        if (count > 50) {
            count = 50;
        }

        ofstream outFile("TopScores.txt");
        if (outFile.is_open()) {
            for (int i = 0; i < count; i++) {
                outFile << names[i] << " " << scores[i] << " " << levels[i] << endl;
            }
            outFile.close();
        }
    }

public:
    HighScores() :score(0), highScore(0) {}

    void setName(string n) {
        name = n;

    }

    void setScore(int s) {
        score = s;
        if (s > highScore) {
            highScore = s;
        }
    }

    string getName()const {
        return name;
    }

    int getScore()const {
        return score;
    }

    int getHighScore()const {
        return highScore;
    }

    void saveInfo(int level) {
        if (score <= 0 || name.empty())
            return;
        ofstream myFile("PlayerInformation.txt", ios::app);
        if (myFile.is_open()) {
            myFile << "-------------------PLAYER'S INFORMATION--------------------" << endl;
            myFile << "Player Name: " << name << endl;
            myFile << "Score: " << score << endl;
            if (level == 1) {
                myFile << "Level: Easy" << endl;
            }
            else if (level == 2) {
                myFile << "Level: Medium" << endl;
            }
            else if (level == 3) {
                myFile << "Level: Hard" << endl;
            }
            myFile.close();
        }

        ofstream myFile2("TopScores.txt", ios::app);
        if (myFile2.is_open()) {
            myFile2 << name << " " << score << " " << level << endl;
            myFile2.close();
            include50Scores();

        }
    }

    void displayHighScores(RenderWindow& window, Font& font) {
        string names[100];
        int scores[100];
        int levels[100];
        int count = 0;

        ifstream file("TopScores.txt");
        if (file.is_open()) {
            while (count < 100 && file >> names[count] >> scores[count] >> levels[count]) {
                count++;
            }
            file.close();

        }

        for (int i = 0; i < count - 1; i++) {
            for (int j = 0; j < count - i - 1; j++) {
                if (scores[j] < scores[j + 1]) {

                    int tempScore = scores[j];
                    scores[j] = scores[j + 1];
                    scores[j + 1] = tempScore;

                    string tempName = names[j];
                    names[j] = names[j + 1];
                    names[j + 1] = tempName;

                    int tempLevel = levels[j];
                    levels[j] = levels[j + 1];
                    levels[j + 1] = tempLevel;
                }
            }
        }
        Text scoretitle;
        scoretitle.setFont(font);
        scoretitle.setString("TOP 50 HIGH SCORES");
        scoretitle.setCharacterSize(40);
        scoretitle.setFillColor(Color::Yellow);
        scoretitle.setPosition(200, 20);

        Text scoreTexts[50];
        int scoreLimit = (count < 50) ? count : 50;

        for (int i = 0; i < scoreLimit; i++) {
            scoreTexts[i].setFont(font);
            string text = to_string(i + 1) + ". " + names[i] + " - " + to_string(scores[i]);
            scoreTexts[i].setString(text);
            scoreTexts[i].setCharacterSize(16);
            scoreTexts[i].setFillColor(Color::White);

            if (i < 25) {
                scoreTexts[i].setPosition(50, 80 + i * 20);
            }
            else {
                scoreTexts[i].setPosition(400, 80 + (i - 25) * 20);
            }
        }

        Text returnText;
        returnText.setFont(font);
        returnText.setString("Press B to go back");
        returnText.setCharacterSize(20);
        returnText.setFillColor(Color::Cyan);
        returnText.setPosition(320, 550);

        while (window.isOpen()) {
            Event event;
            while (window.pollEvent(event)) {
                if (event.type == Event::Closed) {
                    window.close();
                    return;
                }
                if (event.type == Event::KeyPressed && event.key.code == Keyboard::B) {
                    return;
                }
            }

            window.clear();
            window.draw(scoretitle);
            for (int i = 0; i < scoreLimit; i++) {
                window.draw(scoreTexts[i]);
            }
            window.draw(returnText);
            window.display();
        }
    }



    string enterName(RenderWindow& window, Font& font) {
        name = "";

        Text enterName;
        enterName.setFont(font);
        enterName.setString("Type your name (max 8 letters):");
        enterName.setCharacterSize(30);
        enterName.setFillColor(Color::White);
        enterName.setPosition(200, 200);

        Text nameOnScreen;
        nameOnScreen.setFont(font);
        nameOnScreen.setString("_");
        nameOnScreen.setCharacterSize(30);
        nameOnScreen.setFillColor(Color::Yellow);
        nameOnScreen.setPosition(200, 250);

        while (window.isOpen()) {
            Event event;
            while (window.pollEvent(event)) {
                if (event.type == Event::Closed) {
                    window.close();
                    return "";
                }

                if (event.type == Event::KeyPressed) {
                    if (event.key.code == Keyboard::Enter && name.length() > 0) {
                        return name;
                    }
                    else if (name.length() < 8) {
                        if (event.key.code >= Keyboard::A && event.key.code <= Keyboard::Z) {
                            char letter = 'A' + (event.key.code - Keyboard::A);
                            name += letter;
                        }
                    }
                }
            }


            nameOnScreen.setString(name + "_");

            window.clear();
            window.draw(enterName);
            window.draw(nameOnScreen);
            window.display();
        }

        return name;
    }
};

inline void debugHighScore(const HighScores& h) {
    cout << "[DEBUG] Name=" << h.name
        << " Score=" << h.score
        << " HighScore=" << h.highScore << endl;
}

class Menu {
private:
    Texture backgroundTexture;
    Sprite  backgroundSprite;
    Font    menuFont;
    Text    titleText;
    Text    choiceText;
    Text    infoText;
    Text    levelText[3];
    Text    extraText;          

    int selectedLevel = 0;        
    const int windowWidth = 800;
    const int windowHeight = 600;

public:
    Menu() {}

    bool menuloaded(const string& menuFontFile,
        const string& backgroundFile,
        const string& titleFile)
    {
        if (!menuFont.loadFromFile(menuFontFile)) {
            cout << "Can not load font!" << endl;
            return false;
        }
        if (!backgroundTexture.loadFromFile(backgroundFile)) {
            cout << "Failed to load texture B!" << endl;
            return false;
        }

        backgroundSprite.setTexture(backgroundTexture);
        float bgScaleX = (float)windowWidth / backgroundTexture.getSize().x;
        float bgScaleY = (float)windowHeight / backgroundTexture.getSize().y;
        backgroundSprite.setScale(bgScaleX, bgScaleY);


        titleText.setFont(menuFont);
        titleText.setString("GALAXY WARS");
        titleText.setCharacterSize(80);
        titleText.setFillColor(Color::White);
        float titleWidth = titleText.getLocalBounds().width;
        titleText.setPosition((windowWidth - titleWidth) / 2, 100);

   
        choiceText.setFont(menuFont);
        choiceText.setString("Choose Level Difficulty");
        choiceText.setCharacterSize(32);
        choiceText.setFillColor(Color::Magenta);
        float choiceWidth = choiceText.getLocalBounds().width;
        choiceText.setPosition((windowWidth - choiceWidth) / 2, 200);

      
        infoText.setFont(menuFont);
        infoText.setString("1 Easy    2 Medium    3 Hard");
        infoText.setCharacterSize(28);
        infoText.setFillColor(Color::Cyan);
        float infoWidth = infoText.getLocalBounds().width;
        infoText.setPosition((windowWidth - infoWidth) / 2, 250);

        string levels[3] = { "Easy", "Medium", "Hard" };
        for (int i = 0; i < 3; i++) {
            levelText[i].setFont(menuFont);
            levelText[i].setString(levels[i]);
            levelText[i].setCharacterSize(40);
            levelText[i].setFillColor(Color::White);
            float textWidth = levelText[i].getLocalBounds().width;
            levelText[i].setPosition((windowWidth - textWidth) / 2, 300 + i * 70);
        }

  
        extraText.setFont(menuFont);
        extraText.setString("H - High Scores    Esc - Exit");
        extraText.setCharacterSize(24);
        extraText.setFillColor(Color::Yellow);
        float extraWidth = extraText.getLocalBounds().width;
        extraText.setPosition((windowWidth - extraWidth) / 2, 520);

        return true;
    }

    void draw(RenderWindow& window) {
        window.draw(backgroundSprite);
        window.draw(titleText);
        window.draw(choiceText);
        window.draw(infoText);
        for (int i = 0; i < 3; i++) {
            window.draw(levelText[i]);
        }
        window.draw(extraText); 
    }

    void handleInput(Event event) {
        if (event.type == Event::KeyPressed) {
            if (event.key.code == Keyboard::Num1)
                selectedLevel = 1;
            else if (event.key.code == Keyboard::Num2)
                selectedLevel = 2;
            else if (event.key.code == Keyboard::Num3)
                selectedLevel = 3;
            else if (event.key.code == Keyboard::H)
                selectedLevel = -1;   
            else if (event.key.code == Keyboard::Escape)
                selectedLevel = -2;  
        }
    }

    int getLevel() const {
        return selectedLevel;
    }

    bool levelSelected() const {
        return selectedLevel != 0;
    }

    void resetSelection() {
        selectedLevel = 0;
    }
};


class gameover_menu {
    Font& font;
    Text gameOverText;
    Text help;
public:
    gameover_menu(Font& f) : font(f) {
        gameOverText.setFont(font);
        gameOverText.setString("GAME OVER");
        gameOverText.setFillColor(Color::Red);
        gameOverText.setCharacterSize(72);

        FloatRect br = gameOverText.getLocalBounds();
        gameOverText.setPosition((800 - br.width) / 2, 200);

        help.setFont(font);
        help.setCharacterSize(24);
        help.setString("R - Retry    M - Menu    Q - Quit");
        FloatRect hr = help.getLocalBounds();
        help.setPosition((800 - hr.width) / 2, 300);
    }

    void draw(RenderWindow& w) {
        RectangleShape overlay({ 800,600 });
        overlay.setFillColor(Color(0, 0, 0, 150));
        w.draw(overlay);
        w.draw(gameOverText);
        w.draw(help);
    }

    char processKey(const Event& e) {
        if (e.type != Event::KeyPressed) return '\0';
        if (e.key.code == Keyboard::R) return 'R';
        if (e.key.code == Keyboard::M) return 'M';
        if (e.key.code == Keyboard::Q) return 'Q';
        return '\0';
    }
};


class PauseMenu {
    Font& font;
    Text title;
    Text options[3];
    int selectedIndex;

public:
    PauseMenu(Font& f) : font(f), selectedIndex(0) {
        title.setFont(font);
        title.setString("PAUSED");
        title.setCharacterSize(60);
        title.setFillColor(Color::White);
        FloatRect tb = title.getLocalBounds();
        title.setPosition((800 - tb.width) / 2, 150);

        const char* labels[3] = { "Resume", "Restart", "Quit" };
        for (int i = 0; i < 3; i++) {
            options[i].setFont(font);
            options[i].setString(labels[i]);
            options[i].setCharacterSize(32);
            options[i].setFillColor(i == 0 ? Color::Yellow : Color::White);
            FloatRect rb = options[i].getLocalBounds();
            options[i].setPosition((800 - rb.width) / 2, 250 + i * 50);
        }
    }

    void draw(RenderWindow& w) {
        RectangleShape overlay;
        overlay.setSize({ 800.f, 600.f });
        overlay.setFillColor(Color(0, 0, 0, 150));
        w.draw(overlay);
        w.draw(title);
        for (int i = 0; i < 3; i++)
            w.draw(options[i]);
    }

    char handleEvent(const Event& e) {
        if (e.type != Event::KeyPressed)
            return 'N';

        if (e.key.code == Keyboard::Up) {
            selectedIndex--;
            if (selectedIndex < 0) selectedIndex = 2;
            updateColors();
        }
        else if (e.key.code == Keyboard::Down) {
            selectedIndex++;
            if (selectedIndex > 2) selectedIndex = 0;
            updateColors();
        }
        else if (e.key.code == Keyboard::Enter ||
            e.key.code == Keyboard::Return) {

            if (selectedIndex == 0) return 'R';
            if (selectedIndex == 1) return 'S';
            return 'Q';
        }
        else if (e.key.code == Keyboard::P ||
            e.key.code == Keyboard::Escape) {
            return 'R';
        }

        return 'N';
    }

private:
    void updateColors() {
        for (int i = 0; i < 3; i++) {
            if (i == selectedIndex)
                options[i].setFillColor(Color::Yellow);
            else
                options[i].setFillColor(Color::White);
        }
    }
};

