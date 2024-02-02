#ifndef MAINWINDOW_HH
#define MAINWINDOW_HH
#include "player.hh"
#include "card.hh"

#include <QMainWindow>
#include <vector>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QDateTime>
#include <map>

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QLabel>
#include <QTimer>
#include <QScrollArea>
#include <QtWidgets>

// Default settings.
//
// Timer's interval
static const int TIMER_INTERVAL_ms = 1000;

// Card's sizes
const int CARD_WIDTH = 100;
const int CARD_HEIGHT = 100;

// Window's marginals
const int LEFT_MARGIN = 50;
const int TOP_MARGIN = 25;


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

// Handling all actions.
private slots:

    void handleCardButtonClick();
    void startButtonClick();
    void quitButtonClick();
    void turnButtonClick();
    void resetButtonClick();
    void enterClick();

    void boardSelected();

    void cardAmountEdited();

    // Timer's functionality
    void timer_timeout();


private:
    Ui::MainWindow *ui;
    QTimer* timer;

    // creates cards in different threads.
    void create_cards();

    // inits gameboard elements
    void init_gameboard_cards();

    // setups buttons in different threads
    void create_buttons();

    // Edits card picture to be fitting.
    QPixmap set_background_image();

    // Returns numerical value of the str if it's a number.
    unsigned int stoi_with_check(const std::string& str);

    // Checks input
    bool check_card_amount(std::string input);

    void create_players();

    // Updates scoreboard and keeps track of the player who is leading.
    void update_scoreboard();

    // Checks if game has reached game over situation.
    void game_over(Player*, std::vector<Player*>);

    // Pelikorttien kokonaismäärä
    int amount_of_pairs_ = 1;

    // Gameboard sizes.
    // TODO: scaling gameboard by window size.
    int card_rows_ = 6;
    int card_columns_ = 6;

    // For resetting game.
    bool is_reset = false;

    std::vector<Card*> cards_;

    // Card name, button
    std::map<QString, QPushButton*> card_buttons_;

    std::vector<QPushButton*> all_buttons_;

    // Button name, card
    std::map<QString, Card*> randomized_cards_;

    std::vector<Player*> players_;

    // Turned cards during one turn.
    std::vector<Card*> turned_cards_;

    // Guessed buttons during one turn.
    std::vector<QPushButton*> guessed_buttons_;

    // Guessed buttons during the whole game.
    std::vector<QPushButton*> all_guessed_buttons_;

    // Amount of rightly guessed cards.
    int guessed_cards_ = 0;

    // Actions done in one turn (max 2)
    int done_actions_ = 0;

    int turn_num_ = 0;

    Player* in_turn_ = nullptr;

    int player_count_ = 0;

};
#endif // MAINWINDOW_HH
