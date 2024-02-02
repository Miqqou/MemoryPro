#include "mainwindow.hh"
#include "ui_mainwindow.h"
#include "config.hh"

#include <thread>
#include <mutex>
#include <QVariant>
#include <QString>
#include <QRegularExpression>


std::mutex mtx;
std::random_device rd;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , timer(new QTimer(this))
{
    ui->setupUi(this);

    // Setting up elements default state.
    //
    ui->playerList->setHidden(1);

    ui->enterLabel->setText("Enter the amount of players:");
    ui->errorLabel->setText("");

    ui->cardEdit->setMaxLength(6);
    ui->cardEdit->setHidden(1);
    ui->playerEdit->setMaxLength(2);

    ui->startButton->setDisabled(1);
    ui->turnButton->setDisabled(1);
    ui->resetButton->setDisabled(1);

    ui->boardsList->setHidden(1);
    ui->playerList->setHidden(1);
    ui->scoreListLabel->setHidden(1);
    ui->gbSelectionLabel->setHidden(1);

    ui->functionalityGrid->addWidget(ui->playArea, 1, 0);
    ui->functionalityGrid->addWidget(ui->boardsList, 1, 1);
    ui->boardsList->setFixedHeight(650);
    ui->boardsList->setFixedWidth(150);
    ui->functionalityGrid->addWidget(ui->playerList, 1, 2);

    ui->playerList->setFixedHeight(650);
    ui->playerList->setFixedWidth(150);

    ui->enterButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->startButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->functionalityGrid->setRowMinimumHeight(0, 1);
    ui->functionalityGrid->setAlignment(Qt::AlignCenter);

    ui->playArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);


    // Connecting functionality of the elements.
    //
    connect(ui->startButton, &QPushButton::clicked, this,
            &MainWindow::startButtonClick);

    connect(ui->quitButton, &QPushButton::clicked, this,
            &MainWindow::quitButtonClick);

    connect(ui->turnButton, &QPushButton::clicked, this,
            &MainWindow::turnButtonClick);

    connect(ui->cardEdit, &QLineEdit::textEdited, this,
            &MainWindow::cardAmountEdited);

    connect(ui->resetButton, &QPushButton::clicked, this,
            &MainWindow::resetButtonClick);

    connect(timer, &QTimer::timeout, this, &MainWindow::timer_timeout);

    connect(ui->enterButton, &QPushButton::clicked, this,
            &MainWindow::enterClick);

    connect(ui->boardsList, &QListWidget::itemSelectionChanged, this, &MainWindow::boardSelected);
}

MainWindow::~MainWindow()
{
    for (auto player : players_) {
        delete player;
    }

    for (auto card : cards_) {
        delete card;
    }

    for (auto button : all_buttons_) {
        delete button;
    }

    randomized_cards_.clear();
    turned_cards_.clear();

    delete ui;
}

void MainWindow::startButtonClick()
{
    is_reset = false;

    ui->cardEdit->setDisabled(1);
    ui->startButton->setDisabled(1);
    ui->startButton->setHidden(1);
    ui->resetButton->setDisabled(0);
    ui->cardEdit->setHidden(1);
    ui->enterLabel->setHidden(1);
    ui->boardsList->setHidden(0);
    ui->playArea->setHidden(0);
    ui->playArea->setDisabled(0);
    ui->scoreListLabel->setHidden(0);
    ui->gbSelectionLabel->setHidden(0);


    create_players();
    init_gameboard_cards();

    // Starts counting gameplay seconds.
    timer->start(TIMER_INTERVAL_ms);
}

void MainWindow::quitButtonClick()
{
    close();
}

void MainWindow::timer_timeout()
{
    int mins = ui->lcdNumberMin->intValue();
    int secs = ui->lcdNumberSec->intValue();

    if (secs == 59) {
        ui->lcdNumberMin->display(mins + 1);
        ui->lcdNumberSec->display(0);
    }

    else {
        ui->lcdNumberSec->display(secs + 1);
    }
}

void MainWindow::resetButtonClick()
{
    is_reset = true;

    randomized_cards_.clear();
    card_buttons_.clear();

    for (auto card : cards_) {
        delete card;
    }
    cards_.clear();

    for (auto button : all_buttons_) {
        delete button;
    }
    all_buttons_.clear();

    for (size_t i = 0; i < players_.size(); ++i) {
        delete players_.at(i);
    }
    players_.clear();

    guessed_buttons_.clear();
    turned_cards_.clear();

    for (QListWidgetItem* item : ui->playerList->findItems(QString("*"), Qt::MatchWildcard)) {
        delete item;
    }
    ui->playerList->clear();
    ui->boardsList->clear();

    // actions logic
    guessed_cards_ = 0;
    done_actions_ = 0;

    turn_num_ = 0;
    in_turn_ = nullptr;

    // element states
    ui->enterLabel->setText("Enter the amount of players:");
    ui->playerLabel->setText("");
    ui->cardEdit->setText("");

    ui->playerEdit->setHidden(0);
    ui->enterButton->setDisabled(0);
    ui->enterButton->setHidden(0);

    ui->cardEdit->setHidden(1);
    ui->cardEdit->setDisabled(0);

    ui->resetButton->setDisabled(1);
    ui->turnButton->setDisabled(1);

    ui->enterLabel->setHidden(0);

    ui->playArea->setHidden(1);
    ui->playerList->setHidden(1);
    ui->boardsList->setHidden(1);

    ui->scoreListLabel->setHidden(1);
    ui->gbSelectionLabel->setHidden(1);

    // Resetting timer
    timer->stop();
    ui->lcdNumberMin->display(0);
    ui->lcdNumberSec->display(0);
}

void MainWindow::handleCardButtonClick()
{
    QPushButton* pressed_button = static_cast<QPushButton*>(sender());
    QString button_name = pressed_button->objectName();

    // Icon is shown next to the board (in boardlist) which cards have been guessed at some point.
    QListWidgetItem *selectedItem = ui->boardsList->currentItem();
    QIcon undoicon = QIcon::fromTheme("edit-undo");
    selectedItem->setIcon(undoicon);

    // Cards cannot be pressed again.
    pressed_button->setDisabled(1);

    Card* guessed_card = randomized_cards_[button_name];

    // Actions in a turn
    turned_cards_.push_back(guessed_card);
    guessed_buttons_.push_back(pressed_button);
    done_actions_ += 1;

    // When all actions are used disabling all cardbuttons
    if (done_actions_ == 2) {
        ui->playArea->setDisabled(1);
        ui->turnButton->setDisabled(0);
    }

    std::string id = guessed_card->get_id();

    // Hides the card's background image.
    QSize hidden_icon(0, 0);
    pressed_button->setIconSize(hidden_icon);

    // Card's symbol is shown on the button
    pressed_button->setText(QString::fromStdString(id));
    QFont test_font;
    test_font.bold();
    test_font.setPixelSize(32);
    pressed_button->setFont(test_font);
}


void MainWindow::turnButtonClick()
{
    QString button1_name = guessed_buttons_.at(0)->objectName();
    QString button2_name = guessed_buttons_.at(1)->objectName();

    // Right guess.
    if (turned_cards_.at(0)->get_id() == turned_cards_.at(1)->get_id())
    {
        // Guessed buttons are grayed.
        guessed_buttons_.at(0)->setStyleSheet("QPushButton#"
                                              + button1_name
                                              + " {"
                                                "    background-color: gray;"
                                                "    color: black;"
                                                "    border: 2px solid cyan;"
                                                "}"
                                                "");
        guessed_buttons_.at(1)->setStyleSheet("QPushButton#"
                                              + button2_name
                                              + " {"
                                                "    background-color: gray;"
                                                "    color: black;"
                                                "    border: 2px solid cyan;"
                                                "}"
                                                "");


        // buttons are not needed anymore.
        std::map<QString, QPushButton*>::iterator it;
        it = card_buttons_.find(button1_name);
            card_buttons_.erase(it);

        it = card_buttons_.find(button2_name);
            card_buttons_.erase(it);

        in_turn_->add_pair();

        guessed_cards_ += 2;
    }

    // Wrong guess.
    else
    {
        ++turn_num_;

        // Hiding card face.
        QSize shown_icon(CARD_WIDTH, CARD_HEIGHT);
        guessed_buttons_.at(0)->setIconSize(shown_icon);
        guessed_buttons_.at(0)->setText("");
        guessed_buttons_.at(1)->setIconSize(shown_icon);
        guessed_buttons_.at(1)->setText("");
        guessed_buttons_[0]->setDisabled(0);
        guessed_buttons_[1]->setDisabled(0);
    }

    ui->playArea->setDisabled(0);

    // Resetting turn specific parameters.
    turned_cards_.clear();
    guessed_buttons_.clear();
    done_actions_ = 0;
    ui->turnButton->setDisabled(1);

    // When all players have had a turn, go back to first player.
    if (turn_num_ % player_count_ == 0)
    {
        turn_num_ = 0;
    }

    // Updating the player in turn.
    in_turn_ = players_.at(turn_num_);
    QString in_turn_name = QString::fromStdString(in_turn_->get_name());
    ui->playerLabel->setText("Player in turn: " + in_turn_name);

    update_scoreboard();
}

void MainWindow::cardAmountEdited()
{
    std::string amount_of_pairs_str = ui->cardEdit->text().toStdString();
    ui->startButton->setHidden(0);

    // Checking the input.
    if (amount_of_pairs_str.length() > 0) {
        if (check_card_amount(amount_of_pairs_str))
        {
            amount_of_pairs_ = stoi(amount_of_pairs_str);

            ui->startButton->setDisabled(0);
            ui->errorLabel->setText("");
        }
        else
        {
            ui->startButton->setDisabled(1);
            ui->errorLabel->setText("Please input only valid numbers! 1-" + QString::fromStdString(std::to_string(config::MAX_PAIRS)));
            QPalette palet(QColor(250, 1, 1, 255));
            ui->errorLabel->setPalette(palet);
        }
    }

    else {
        ui->startButton->setDisabled(1);
        ui->errorLabel->setText("");
    }
}


void MainWindow::enterClick()
{
    std::string player_amount_str = ui->playerEdit->text().toStdString();

    // Checking the input.
    int value = stoi_with_check(player_amount_str);
    if (value > 1) {

        player_count_ = value;

        ui->playerEdit->setHidden(1);
        ui->enterButton->setDisabled(1);
        ui->enterButton->setHidden(1);

        ui->errorLabel->setText("");
        ui->enterLabel->setText("Enter the amount of pairs:");
        ui->cardEdit->setHidden(0);
    }
    else {
        ui->errorLabel->setText("Valid numbers 2-99!");
    }
}

void MainWindow::boardSelected(){

    if (!is_reset) {

        // First clearing the board
        QLayout* layout = ui->playArea->layout();
        while (auto item = layout->takeAt(0)) {
            QWidget* widget = item->widget();
            if (widget) {
                layout->removeWidget(widget);
                widget->setParent(nullptr);
            }
            delete item;
        }

        QListWidgetItem *selectedItem = ui->boardsList->currentItem();
        int board_num = (selectedItem->text()).toInt();

        int cards_per_board = card_columns_*card_rows_;

        // Current implementation always draws the coordinates.
        ui->hLayout->addWidget(new QLabel("y/x"), 0, 0);

        int row_num = 1;
        int column_num = 1;
        for (int i = 0; i < cards_per_board; ++i) {

            if (row_num == 1 && column_num != card_columns_+1) {
                ui->hLayout->addWidget(new QLabel(QString::number(column_num)), 0, column_num);
                QWidget* widget = ui->hLayout->itemAtPosition(0, column_num)->widget();
                if (widget) {
                    widget->setStyleSheet("border: 2px solid black;");
                }
            }
            if (i % card_columns_ == 0 and i > 0 ) {
                ui->hLayout->addWidget(new QLabel(QString::number(row_num)), row_num, 0);
                QWidget* widget = ui->hLayout->itemAtPosition(row_num, 0)->widget();
                if (widget) {
                    widget->setStyleSheet("border: 2px solid black;");
                }
                row_num += 1;
                column_num = 1;
            }
            if (board_num * cards_per_board + i < 2* amount_of_pairs_) {
                QPushButton* newButton = all_buttons_[board_num * cards_per_board + i];
                ui->hLayout->addWidget(newButton, row_num, column_num);
            }
            ++column_num;
        }

        ui->hLayout->addWidget(new QLabel(QString::number(row_num)), row_num, 0);
        QWidget* widget = ui->hLayout->itemAtPosition(row_num, 0)->widget();
        if (widget) {
            widget->setStyleSheet("border: 2px solid black;");
        }
    }
}

QPixmap MainWindow::set_background_image()
{
    // Converting image (png) to a pixmap
    QPixmap image(QString::fromStdString(config::BG_IMAGE_PATH));

    // Scaling the pixmap
    image = image.scaled(CARD_WIDTH, CARD_HEIGHT);

    return image;
}

void init_cards(std::vector<Card*> &cards_, char starting_point_letter, int amount, int range)
{
    int i = 0;
    char c = starting_point_letter;

    std::vector<Card*> threads_cards;

    // Creating two cards with the same id.
    // 26 letters and 999 numbers. e.g. a999.
    // need to generate only for selected amount of pairs.
    int generated_pairs = 0;
    while (i < 1000)
    {
        while (c <= 'z') {
            Card* card1 = new Card();
            Card* card2 = new Card();

            card1->set_id(c+std::to_string(i));
            card2->set_id(c+std::to_string(i));

            threads_cards.emplace_back(card1);
            threads_cards.emplace_back(card2);

            ++generated_pairs;

            ++c;

            // Exiting when enough cards has been generated.
            if (generated_pairs >= amount) {
                i = 1000;
                break;
            }
        }
        ++i;
        c = starting_point_letter;
    }

    // Thread safe editing own specific memory location of the global cards vector.
    for (int i = 0; i < threads_cards.size(); ++i) {
        cards_[range + i] = threads_cards[i];
    }
}

unsigned int MainWindow::stoi_with_check(const std::string& str)
{
    if (str.length() > 0) {
        bool is_numeric = std::all_of(str.begin(), str.end(), [](char c) { return std::isdigit(c); });

        return is_numeric ? std::stoi(str) : 0;
    }
    else {
        return 0;
    }
}

bool MainWindow::check_card_amount(std::string input)
{
    bool is_valid = false;

    // Checks if numerical
    if (stoi_with_check(input) != 0) {
        int amount = stoi(input);

        if (amount <= config::MAX_PAIRS) {
            is_valid = true;
        }
    }
    return is_valid;
}

void MainWindow::create_players()
{
    std::string player_name;

    for (int i = 0; i < player_count_; ++i)
    {
        // Creating player entitys.
        std::string player_name = "player" + std::to_string(i+1);
        Player* player = new Player(player_name);
        players_.push_back(player);
        std::string points = std::to_string(players_.at(i)->number_of_pairs());

        // Data structure for the QList
        std::pair<QString, int> playerData = std::make_pair(QString::fromStdString(player_name), 0);
        QListWidgetItem* item = new QListWidgetItem(QString("%1 : %2").arg(playerData.first).arg(playerData.second));
        item->setData(Qt::UserRole, QVariant::fromValue(playerData));
        ui->playerList->addItem(item);

        ui->playerList->setVisible(1);
    }
}

// create threads
void MainWindow::create_cards(){

    std::vector<char> alphabet(26);
    std::iota(alphabet.begin(), alphabet.end(), 'a');

    int amount = alphabet.size() / config::num_of_threads;

    int pairs_per_thread =  amount_of_pairs_ / config::num_of_threads;
    int divident = amount_of_pairs_ % config::num_of_threads;
    int extra =  0;
    if (divident != 0) {
        extra = divident;
    }

    int use_threads = config::num_of_threads;
    if (amount_of_pairs_ < config::num_of_threads) {
        use_threads = 1;
    }

    std::vector<std::thread> threads;
    for ( int t = 0; t < use_threads; t++ ) {
        // last thread handles extra cards.
        if (t == use_threads - 1) {
            threads.emplace_back(init_cards, std::ref(cards_), alphabet.at(t*amount), pairs_per_thread + extra, t * (2 * pairs_per_thread));
        }
        else {
            threads.emplace_back(init_cards, std::ref(cards_), alphabet.at(t*amount), pairs_per_thread, t * (2 * pairs_per_thread));
        }
    }

    for ( auto& t : threads ) {
        t.join();
    }
}

void create_buttons_threading(std::vector<Card*> &cards_, std::vector<QPushButton*> &all_buttons_, std::map<QString, Card*> &randomized_cards_, std::map<QString, QPushButton*> &card_buttons_, int amount, int start_point,  QPixmap image) {

    std::map<QString, Card*> randomized_cards_thread = {};
    std::map<QString, QPushButton*> card_buttons_thread = {};

    for (int i = 0; i < amount; ++i) {

        QPushButton* pushButton = all_buttons_[start_point + i];

        pushButton->setFixedWidth(CARD_WIDTH);
        pushButton->setFixedHeight(CARD_HEIGHT);
        pushButton->setIcon(image);
        pushButton->setIconSize(QSize(image.width(), image.height()));

        QString name = "button" + QString::number(start_point + i);
        pushButton->setObjectName(name);

        // Thread safe editing own specific memory location of the global buttons vector.
        all_buttons_[start_point + i] = pushButton;

        // Have to use thread specific maps.
        randomized_cards_thread[name] = cards_[start_point + i];
        card_buttons_thread[name] = pushButton;
    }

    // Maps have to be edited 1 thread at the time because not thread safe.
    mtx.lock();
    randomized_cards_.insert(randomized_cards_thread.begin(), randomized_cards_thread.end());
    card_buttons_.insert(card_buttons_thread.begin(), card_buttons_thread.end());
    mtx.unlock();
}

void MainWindow::create_buttons() {
    // Asetetaan kortille taustakuva
    QPixmap image = set_background_image();

    int amount = (2 * amount_of_pairs_) / config::num_of_threads;

    int divident = (2 * amount_of_pairs_) % config::num_of_threads;
    int extra = 0;
    if (divident != 0) {
        extra = divident;
    }

    std::vector<std::thread> threads;
    for ( int t = 0; t < config::num_of_threads; t++ ) {
        if (t == config::num_of_threads - 1) {
            threads.emplace_back(create_buttons_threading, std::ref(cards_), std::ref(all_buttons_), std::ref(randomized_cards_), std::ref(card_buttons_), amount + extra, t * amount, image);
        }
        else {
            threads.emplace_back(create_buttons_threading, std::ref(cards_), std::ref(all_buttons_), std::ref(randomized_cards_), std::ref(card_buttons_), amount, t * amount, image);
        }
    }

    for ( auto& t : threads ) {
        t.join();
    }
}


void MainWindow::init_gameboard_cards()
{
    // Layout settings
    ui->playArea->setGeometry(20, 150, 1600, 1000);
    QGridLayout* hLayout = ui->hLayout;

    ui->playArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    hLayout->setSizeConstraint(QLayout::SetFixedSize);
    hLayout->setSpacing(0);
    hLayout->setContentsMargins(0,0,0,0);
    ui->playArea->setLayout(hLayout);

    // resizing vectors in advance so that memory location isnt changed when different threads are adding elements to them.
    cards_.resize(2*amount_of_pairs_);
    all_buttons_.resize(2*amount_of_pairs_);
    create_cards();


    // Shuffle cards, so their position is always random.
    std::shuffle(cards_.begin(), cards_.end(), std::default_random_engine {rd()});

    // Have to create QObjects in the main thread.
    for (int i = 0; i < 2 * amount_of_pairs_; ++i) {
        QPushButton* pushButton = new QPushButton(this);
        pushButton->setStyleSheet("QPushButton#button"
                                  + QString::number(i)
                                  + " {"
                                  "    background-color: white;"
                                  "    color: black;"
                                  "    border: 2px solid cyan;"
                                  "}"
                                  "");
        all_buttons_[i] = pushButton;
    }
    // Updating all the information for the buttons in threads.
    create_buttons();

    int num_of_boards = 0;
    int row_num = 0;
    int column_num = 0;
    for (int i = 0; i < 2 * amount_of_pairs_; ++i) {
        QPushButton* pushButton = all_buttons_[i];

        if (i % card_columns_ == 0 and i > 0 ) {
            row_num += 1;
            column_num = 0;
        }

        connect(pushButton, &QPushButton::clicked, this,
                &MainWindow::handleCardButtonClick);

        // Creating boardlist
        if (i % (card_columns_*card_rows_) == 0) {
            QListWidgetItem *item = new QListWidgetItem(QString::fromStdString(std::to_string(num_of_boards)));
            ui->boardsList->addItem(item);
            num_of_boards += 1;
        }

        ++column_num;
    }
    ui->boardsList->setCurrentRow(0);

    // First player starts.
    in_turn_ = players_.at(0);
    QString in_turn_name = QString::fromStdString(in_turn_->get_name());
    ui->playerLabel->setText("Player in turn: " + in_turn_name);
}

// For comparing players points from QList.
bool comparePlayersByPoints(QListWidgetItem* item1, QListWidgetItem* item2) {
    QVariant data1 = item1->data(Qt::UserRole);
    QVariant data2 = item2->data(Qt::UserRole);

    std::pair<QString, int> playerData1 = data1.value<std::pair<QString, int>>();
    std::pair<QString, int> playerData2 = data2.value<std::pair<QString, int>>();

    if (playerData1.second == playerData2.second) {
        // Extract numeric parts from player names
        QString name1 = playerData1.first.replace("player", "");
        QString name2 = playerData2.first.replace("player", "");

        // Compare numeric parts as integers
        int num1 = name1.toInt();
        int num2 = name2.toInt();

        return num1 > num2;
    }
    return playerData1.second > playerData2.second;
}

void MainWindow::update_scoreboard()
{
    Player* winning_player = players_.at(0);
    std::vector<Player*> tied_players;
    tied_players.clear();

    // Update whole playerlist.
    for (std::string::size_type num = 0; num < players_.size(); ++num) {

        Player* player = players_.at(num);
        std::string player_name = player->get_name();
        int player_points = player->number_of_pairs();

        QListWidgetItem* item = ui->playerList->item(num);
        if (item) {
            QVariant data = item->data(Qt::UserRole);
            std::pair<QString, int> playerData = data.value<std::pair<QString, int>>();

            // Update the points
            playerData.second = player_points;

            // Update the text of the item
            item->setText(QString("%1 : %2").arg(playerData.first).arg(playerData.second));

            // Update the data of the item
            item->setData(Qt::UserRole, QVariant::fromValue(playerData));
        }

        // Updates currently winning player
        if ( player_points > winning_player->number_of_pairs() ) {
            winning_player = player;
        }

        // Adds players to tied players list.
        else if ( player_points  == winning_player->number_of_pairs() &&
                  player_name != winning_player->get_name() )
        {
            tied_players.push_back(player);
            tied_players.push_back(winning_player);
        }
    }

    // Bubble sorting scorelist.
    // TODO: Fix sorting
    /*int itemCount = ui->playerList->count();
    for (int i = 0; i < itemCount - 1; ++i) {
        for (int j = 0; j < itemCount - i - 1; ++j) {
            QListWidgetItem* current_item = ui->playerList->item(j);
            QListWidgetItem* next_item = ui->playerList->item(j + 1);

            if (!comparePlayersByPoints(current_item, next_item)) {
                ui->playerList->takeItem(j);
                ui->playerList->insertItem(j + 1, current_item);
            }
        }
    }*/

    // Checks if gameover situation.
    game_over(winning_player, tied_players);
}

void MainWindow::game_over(Player* winning_player,
                           std::vector<Player*>tied_players)
{

    // End of the game when all the cards have been guessed
    if ( guessed_cards_ / 2 == amount_of_pairs_)
    {
        QString end_text = "";

        // Points of the tied players.
        if ( tied_players.size() > 1 )
        {
            std::string pairs_amount = std::to_string(tied_players.at(0)->
                                                number_of_pairs());

            std::string player_amount = std::to_string(tied_players.size());

            end_text = "Tie of "
                    + QString::fromStdString(player_amount)
                    + " players with "
                    + QString::fromStdString(pairs_amount)
                    + " pairs.";

            ui->playerLabel->setText(end_text);
        }

        // Winners points
        else
        {
            std::string pairs_amount = std::to_string(winning_player->
                                                      number_of_pairs());

            end_text = QString::fromStdString(winning_player->
                                                      get_name())
                    + " has won with "
                    +  QString::fromStdString(pairs_amount)
                    +  " pairs.";

            ui->playerLabel->setText(end_text);
        }

        // Gameplay timer is stopped.
        timer->stop();
    }
}

