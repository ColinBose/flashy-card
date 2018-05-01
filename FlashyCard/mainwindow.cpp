#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "cardlist.h"
#define SERVERIP "127.0.0.1"
#define SERVERPORT "7000"
#define CONFIGFILE "conf.txt"
StudyClock * c;
StudyClock * countdownClock;
MultiManager networkedManager;
QColor bgColours[10] = {Qt::green, Qt::red, Qt::blue, Qt::cyan, Qt::gray, Qt::yellow, Qt::darkBlue, Qt::darkMagenta, Qt::darkGreen, Qt::darkCyan};
QPushButton * gameButtons[16];
QStringList colors({"red", "green","blue", "cyan","purple"});
QString buttonColours[7] = { "* { background-color: rgb(0,99,0)}","* { background-color: rgb(33,255,33)}",
                             "* {background-color: rgb(255,255,0)}","* { background-color: rgb(255,200,50)}",
                             "* {background-color: rgb(255,90,50)}","* { background-color: rgb(180,35,0)}"
                           ,"* { background-color: rgb(255,255,255)}"};




MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    returnToMain();
    //studyTime = QTime::fromString("000000", "HHmmss");
    c = new StudyClock(this);
    countdownClock = new StudyClock(this);
    networkedManager.setData(&db, this);
    loadGameButtons();
    loadConfig();
  //  loadGame();
}


void * networkWorker(void *args){
    networkedManager.readLoop();
}


void MainWindow::testFunction(char buff[]){

}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::returnToMain(){
    setPage(MAINPAGE);
    db.checkDailyReset();

    setDue();
    updateStats();
    if(!connected){
        ui->mainMulti->setEnabled(false);
        ui->friendButton->setEnabled(false);
        ui->mainLogin->setEnabled(false);
    }
    else{
        ui->mainMulti->setEnabled(true);
        ui->mainLogin->setEnabled(true);
    }
    popDeckTable();



}
void MainWindow::updateStats(){
    db.addDaysToStats();
    StatInfo stats;
    QList<statCard> cardList;
    QSqlQuery result = db.getAllActive();
    while(result.next()){
        statCard s;
        s.interval = result.value(0).toInt();
        s.avaerage = result.value(1).toInt();
        s.numDone = result.value(2).toInt();
        cardList.push_back(s);
    }

    stats.getCardLevels(cardList);
    db.updateCardLevels(stats.strong, stats.learning, stats.longLearning, stats.longStruggle,stats.newStrong, stats.newWeak);
}

void MainWindow::setDue(){
    QSqlQuery result = db.getDecks();
    int currentDue, newDue;
    while(result.next()){
        QString deck = result.value("DeckName").toString();
        currentDue = db.getCurrentDue(deck);
        if(currentDue < 0){
            qDebug() << "Error with get current due";
            continue;
        }
        newDue = db.getDailyDue(deck);
        newDue -= currentDue;
        if(newDue < 0)
            newDue = 0;
        db.setDailyDue(deck, newDue);


    }
}

void MainWindow::popDeckTable(){
    QSqlQuery result = db.getDecks();
    QStringList header, items;
    header << "Deck" << "Due";
    items << "DeckName" << "DName";
    populateATable(ui->mainDeckView,header, items,result, true);
}
void MainWindow::populateATable(QTableWidget * table, QStringList headers, QStringList items, QSqlQuery result, bool stretch){
    table->clear();
    table->setRowCount(0);

    if(headers.length() != items.length())
        return;

    if(stretch)
        table->horizontalHeader()->setStretchLastSection(true);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->verticalHeader()->hide();
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    int colCount = headers.size();
    table->setColumnCount(colCount);
    if(headers.length() != 0){
        table->setHorizontalHeaderLabels(headers);
    }
    int x = 0;
    while(result.next()){
        table->insertRow(x);
        for(int i = 0; i < colCount; i++){
            QString t = result.value(items.at(i)).toString();
            if(t.length() == 0)
                t = "0";
            table->setItem(x, i, new QTableWidgetItem(t));


        }


        x++;
    }
}

void MainWindow::on_mainDeckManager_2_clicked()
{
    ui->stackedWidget->setCurrentIndex(CREATEDECKPAGE);
}

void MainWindow::on_createDeckButton_clicked()
{
    int maxInterval, dailyUpdates;
    QString deckName = ui->createDeckEdit->text();
    if(deckName.length() == 0)
        return;
    maxInterval = ui->createInterval->text().toInt();
    if(maxInterval == 0)
        return;
    dailyUpdates = ui->createUpdates->text().toInt();
    if(dailyUpdates == 0)
        return;
    if(db.createDeck(ui->createDeckEdit->text(), maxInterval, dailyUpdates)){
        //DO A POPUP HERE
        ui->createDeckEdit->setText("");
        QMessageBox Msgbox;
        Msgbox.setText("Deck Created Successfully!");
        Msgbox.exec();
    }
    else{
        QMessageBox Msgbox;
        Msgbox.setText("Error Creating Deck");
        Msgbox.exec();
    }

}

void MainWindow::on_addCardButton_clicked()
{
    QString front, back;
    front = ui->addCardFront->text();
    back = ui->addCardBack->text();
    if(front.length() == 0 || back.length() == 0)
        return;
    deckM.addCard(db, front, back);
    ui->addCardBack->setText("");
    ui->addCardFront->setText("");
}

void MainWindow::on_mainDeckManager_clicked()
{
    int row = ui->mainDeckView->selectionModel()->currentIndex().row();
    if(row == -1){
        return;
    }
    QString deckName = ui->mainDeckView->item(row, 0)->text();
    deckM.loadDeck(deckName);
    ui->deckTitle->setText(deckName);
    curDeck = deckName;
    setPage(DECKMANAGER);
}
void MainWindow::setPage(int i){
    ui->stackedWidget->setCurrentIndex(i);

}

void MainWindow::on_deckManagerAddCard_clicked()
{
    ui->deckTitle->setText(deckM.getDeck());
    setPage(ADDCARD);
}

void MainWindow::on_inputFileSelect_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Image"), "/", tr("All Files(*)"));
    deckM.setFileName(fileName);
    ui->inputCheckInput->setEnabled(true);
    ui->importFileLabel->setText(fileName);
}

void MainWindow::on_deckManagerImport_clicked()
{
    setPage(IMPORTPAGE);
    ui->inputCheckInput->setEnabled(false);
    ui->importSeperatorEdit->setEnabled(false);
    ui->importComboBack->setEnabled(false);
    ui->importComboFront->setEnabled(false);
    ui->importComboUnit->setEnabled(false);
    ui->importImportButotn->setEnabled(false);
    ui->importComboSound->setEnabled(false);
}

void MainWindow::on_inputCheckInput_clicked()
{
    int index = ui->importSelector->currentIndex();
    QChar sep;
    QString output;
    if(index == 0)
        sep = '\t';
    else if(index == 1)
        sep = '-';
    else
        sep = ui->importSeperatorEdit->text().at(0);
    QStringList parts = deckM.checkLine(sep);

    if(parts.length() == 0){
        qDebug() << "Error opening or reading file";
        return;
    }
    ui->importComboBack->clear();
    ui->importComboFront->clear();
    ui->importComboUnit->clear();
    ui->importComboSound->clear();
    ui->importComboBack->addItem("NONE");
    ui->importComboFront->addItem("NONE");
    ui->importComboUnit->addItem("NONE");
    ui->importComboSound->addItem("NONE");
    for(int i = 0; i < parts.length(); i++){
        ui->importComboBack->addItem(parts.at(i));
        ui->importComboFront->addItem(parts.at(i));
        ui->importComboUnit->addItem(parts.at(i));
        ui->importComboSound->addItem(parts.at(i));
    }
    QString coloured;

    for(int i =0; i < parts.length(); i++){
        coloured += "<font color='" + colors.at(i%5) +"'>";
        coloured += parts.at(i);
        coloured += "</font>\t";
    }
    ui->importSampleBreakdown->setText(coloured);
    ui->importComboBack->setEnabled(true);
    ui->importComboFront->setEnabled(true);
    ui->importComboUnit->setEnabled(true);
    ui->importComboSound->setEnabled(true);

    ui->importImportButotn->setEnabled(true);
}



void MainWindow::on_importSelector_currentIndexChanged(int index)
{
    if(index == 2)
        ui->importSeperatorEdit->setEnabled(true);
    else
        ui->importSeperatorEdit->setEnabled(false);
}

void MainWindow::on_importImportButotn_clicked()
{
    int front,back,unit,sound;
    front = ui->importComboFront->currentIndex();
    back = ui->importComboBack->currentIndex();
    unit = ui->importComboUnit->currentIndex();
    sound = ui->importComboSound->currentIndex();
    if(front == 0 || back == 0){
        displayMessage("Ensure front and back are both set");
        return;

    }
    deckM.doImport(db,front,back,unit,sound);
}

void MainWindow::on_mainStudy_clicked()
{


    int row = ui->mainDeckView->selectionModel()->currentIndex().row();
    if(row == -1){
        return;
    }
    QString deckName = ui->mainDeckView->item(row, 0)->text();
    curDeck = deckName;
    study.setDeck(deckName);

    loadDeckSummary();

}
void MainWindow::loadDeckSummary(){
    QString deck = study.getDeckName();
    int due, total, seen;

    due = db.getDueTotal(deck);
    total = db.getCardTotal(deck);
    seen = db.getSeenTotal(deck);
    db.updateMaxInterval(deck);
    ui->deckTitle->setText(deck);
    ui->deckSumSeen->setText(QString::number(seen));
    ui->deckSumToday->setText(QString::number(due));
    ui->deckSumTotal->setText(QString::number(total));
    setPage(DECKSUMMARY);
    studyImproved = 0;
    studyDeclined = 0;
}
void MainWindow::setTodayStat(QSqlQuery result){
    if(result.next()){
        attempted = result.value("Attempted").toInt();
        correct = result.value("Correct").toInt();
        avgPercent = result.value("AvgPercent").toInt();
        newCards = result.value("New").toInt();
        studyTime = QTime::fromString(result.value("Time").toString(),Qt::ISODate);
        if(studyTime.isNull()){
            studyTime.setHMS(0,0,0,0);
        }
    }
    else{
        attempted = 0;
        correct = 0;
        avgPercent = 0;
        newCards = 0;
        //studyTime = QTime::fromString("000000", "HHmmss");
        studyTime.setHMS(0,0,0,0);
    }
}

void MainWindow::on_deckSummaryStudy_clicked()
{
    if(!study.setSession(db))
        return;

    study.loadGrammar(db);
    QString deckName = ui->deckTitle->text();
    int interval = db.getInterval(deckName);
    study.setMaxInterval(interval);

    QSqlQuery result = db.getTodaysStats();
    setTodayStat(result);
    //this->resize(800, 750);
    setPage(STUDY);

    ui->studyMultiBox->setHidden(true);

  //  if(study.isSentences())
    //    ui->studySentenceCheck->setChecked(true);
    //else
      //  ui->studySentenceCheck->setChecked(false);

    singleStudyLoadQuestion();
    threadStarter();
}

void MainWindow::on_studyAnswerEdit_returnPressed()
{
    //eval answer
    on_studyAnswerButton_clicked();

}

void MainWindow::on_studyAnswerButton_clicked()
{
    int answerVal;
    QString answerString;

    if(question){
        if(answerLock)
            return;
        QString answer = ui->studyAnswerEdit->text();
        if(answer.length() == 0)
            return;

        bool sentence;
        sentence = study.checkSentence(curCard->back);
        if(sentence)
            qDebug() << "Doing sentence eval";
        answerString = study.evalAnswer(*curCard,answer, sentence, &answerVal, ui->studyHintLabel->text());
        dealWithAnswer(answer, answerVal, answerString);

        question = false;
        if(multiStudy && !networkedManager.isIndependant()){
            ui->studyAnswerButton->setEnabled(false);
            answerLock = true;
        }

    }
    else{
        if(!multiStudy || networkedManager.isIndependant())
            singleStudyLoadQuestion();
    }



}
void MainWindow::singleStudyLoadQuestion(){
    if(!study.getNext(&curCard)){

        study.clean();
        if(!multiStudy){
            returnToMain();
        }else{
            clearAllStudy();
            //CLEAN UP ALL labels etc here
        }
        return;
    }
    loadQuestion(curCard);
}
void MainWindow::multiLoadQuestion(int cardNum){
    if(cardNum == -2){
        ui->studyAnswerButton->setEnabled(false);
        answerLock = true;
        return;
    }
    if(networkedManager.isIndependant()){
        singleStudyLoadQuestion();
        return;
    }
    if(study.complete()){
        study.clean();
        clearAllStudy();
        networkedManager.sendUserDone();
    }

    //No cards in reserve and current card already completed
    if(!study.getMultiNext(&curCard, cardNum)){
        networkedManager.sendFakeResponse();
        ui->studyFront->setText("No current card, wait for others to finish current round");
    }
    else{
        sleep(1);
        loadQuestion(curCard);
    }
    setCountdown(networkedManager.interval+2);

}
void MainWindow::setCountdown(int sec){
    //studyTime = QTime::fromString("000000", "HHmmss");
    QString time = "0000";
    if(sec < 10)
        time += "0";
    time += QString::number(sec);
    countDown = QTime::fromString(time, "HHmmss");
}

void MainWindow::addSecToClock(){
    studyTime = studyTime.addSecs(1);
    ui->studyElapsedTIme->setText("Time Studied: " + studyTime.toString(Qt::ISODate));
}
void MainWindow::removeSecFromClock(){
    if(countDown.second() <= 0)
        return;
    countDown = countDown.addSecs(-1);
    ui->studyCountDown->setText("Current Round: " + countDown.toString("ss"));
}

void MainWindow::loadQuestion(Card * c){
    hideStudyAnswer();
    ui->studyFront->setText(c->front);
    ui->studyHintEdit->setText(c->hint);
    ui->studyAnswerButton->setEnabled(true);
    answerLock = false;
    ui->studyFront->setText(curCard->front);
    if(ui->studyHintCheck->isChecked()){
        QString hint = study.loadHint(*curCard, true);

        hint = addHintToHint(hint,curCard->hint, curCard->back);
        ui->studyHintLabel->setText(hint);
        ui->studyHintEdit->setText(curCard->hint);
    }
    ui->studyRemaining->setText(QString::number(study.remain()));
    ui->studyAnswerEdit->setText("");
    ui->studyAnswerLabel->setText("");

    ui->studyPastAttempts->setText(QString::number(c->numDone));
    ui->studyPastAverage->setText(QString::number(c->past));
    ui->studyPastInterval->setText(QString::number(c->interval));

    QString otherBack = "* { background-color: rgb(0,0,0) }";

    ui->studyPrevLevel->setStyleSheet(buttonColours[curCard->level]);
    ui->studyNewLevel->setStyleSheet(otherBack);
    ui->studyAnswerPercentLabel->setText("Score:");
    question = true;


}
void MainWindow::hideStudyAnswer(){
    ui->studyNewAverage->setText("");
    ui->studyNewInterval->setText("");
    ui->studyAnswerLabel->setText("");
    ui->studyAnswerEdit->setText("");
}
void MainWindow::showStudyAnswer(){

}

void MainWindow::on_studyHintButton_clicked()
{
    QString hint = ui->studyHintEdit->text();
    //if(hint.length()==0)
      //  return;
    curCard->hint = hint;
    if(!db.updateHint(hint,curCard->code)){
        qDebug() << "Error adding hint";
    }

}
void MainWindow::dealWithAnswer(QString answer, int ansVal, QString answerString){

    //colourAnswer(answer);
    ui->studyAnswerLabel->setText(answerString);

    bool rem =  updateScoringDue(ansVal);
    if(networkedManager.isIndependant() && networkedManager.isSolo())
        return;
    if(multiStudy){
        if(rem){
            if(networkedManager.isIndependant()){
                networkedManager.sendIndTick();
            }
            else{

                networkedManager.sendAnswer(curCard->cardNum, ansVal,'1');
            }
        }
        else{
            if(!networkedManager.isIndependant())
                networkedManager.sendAnswer(curCard->cardNum, ansVal,'0');
        }
    }

}
void MainWindow::updateAverage(int curScore){
    if(attempted == 0){
        avgPercent = curScore;
        return;
    }
    int total = attempted * avgPercent;
    total += curScore;
    double div = (double)total / (attempted + 1);
    avgPercent = (int)round(div);

}

bool MainWindow::updateScoringDue(int ans){
    int newInterval;
    int newAverage;
    int newLevel;
    int newAttempts;
    newLevel = -1;
    bool ret =false;
    updateAverage(ans);
    attempted++;
    if(!curCard->prevDone){
        if(curCard->newCard && ans < 100){
            //maybe nothing here
        }
        else{
            newAverage = study.getAverage(curCard,ans);
            newInterval = study.getInterval(curCard,ans);

            if(newInterval > study.maxInterval)
                newInterval = study.maxInterval;

            newAttempts = curCard->numDone + 1;
            int newStage = floor(newInterval/10);
            if(newStage < curCard->stage)
                newStage = curCard->stage;
            if(!db.updateDue(newAverage,newInterval,newAttempts,newStage,curCard->code)){
                qDebug() << "Error updating card";
            }
            curCard->numDone = newAttempts;
            curCard->past = newAverage;
            curCard->interval = newInterval;
            newLevel = genLevel(newAverage,newInterval,newAttempts);
            if(!curCard->newCard){

                if(newLevel < curCard->level){
                    studyImproved++;
                }else if(newLevel > curCard->level){
                    studyDeclined++;

                }
            }

        }
        
    }
    if(newLevel == -1)
        newLevel = genLevel(curCard->past, curCard->interval, curCard->numDone);
    ui->studyNewLevel->setStyleSheet(buttonColours[newLevel]);
    ui->studyNewAverage->setText(QString::number(curCard->past));
    ui->studyNewInterval->setText(QString::number(curCard->interval));
    ui->studyImproved->setText(QString::number(studyImproved));
    ui->studyDeclined->setText(QString::number(studyDeclined));

    int minPass = ui->studyScoreSlider->value();
    QString answerValString = QString::number(ans);
    if(ans >= minPass){
        answerValString = greenString(answerValString);
        if(curCard->newCard && ans == 100){
            newCards++;
            correct++;
            study.remove();
            ret = true;
        }
        else if(!curCard->newCard){
            correct++;
            study.remove();
            ret = true;
        }
        else if(curCard->newCard)
            incrementTime(77);
    }else{
        answerValString = redString(answerValString);
        if(!curCard->newCard)
            curCard->prevDone = true;
        incrementTime(ans);
    }
    ui->studyAnswerPercentLabel->setText("Score: " + answerValString);
    db.updateStats(attempted, correct, avgPercent, newCards, studyTime );
    //For All
    return ret;

}


void MainWindow::colourAnswer(QString answer){
    QString colouredAns = "";
    for(int i = 0; i < answer.length(); i++){
        //deal with out of range here somehow.
        if(answer.at(i) == curCard->back.at(i)){
            //if(curCard->hint.at())
        }
    }
}
void MainWindow::incrementTime(int ans){
    QDateTime currentTime = QDateTime::currentDateTime();
    if(ans < 60){
        curCard->lastTry = currentTime.addSecs(360);
    }
    else if(ans < 80){
        curCard->lastTry = currentTime.addSecs(180);
    }else{
        curCard->lastTry = currentTime.addSecs(120);
    }

}

void MainWindow::on_studyScoreSlider_sliderMoved(int position)
{
    ui->studyMinScoreLabel->setText("Minimum Score:" + QString::number(position));
}

void MainWindow::on_deckManagerCollection_clicked()
{
    QString deck = ui->deckTitle->text();
    ui->deckTitle->setText(deck);
    setPage(COLLECTION);
    loadCollection(deck);
}
void MainWindow::loadCollection(QString deck){
    QSqlQuery result;
    QStringList units, cardList, activeList;
    miniList.clear();

    result = db.getAllCards(deck);
    units = db.getUnits(deck);
    while(result.next()){
        cardList.push_back(result.value("Front").toString());
        miniCard m;
        m.front = result.value("Front").toString();
        if(result.value("Inactive") == "NO")
            m.active = true;
        m.unit = result.value("Unit").toInt();
        miniList.push_back(m);


    }

    activeList = setActiveInactive(units,miniList);
    populateUnitList(units, activeList);

}
void MainWindow::populateUnitList(QStringList units, QStringList actives){
    if(units.length() != actives.length()){
        qDebug() << "Error with list length";
        return;

    }
    lastRowSelected = -1;
    ui->collectionUnitList->clear();
    ui->collectionUnitList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->collectionUnitList->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->collectionUnitList->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->collectionUnitList->setColumnCount(1);
    ui->collectionUnitList->horizontalHeader()->setStretchLastSection(true);
    ui->collectionUnitList->verticalHeader()->hide();
    QStringList headers;
    headers.push_back("Unit");
    ui->collectionUnitList->setHorizontalHeaderLabels(headers);
    for(int i = 0; i < units.length(); i++){
        QString curUnit = units[i];
        ui->collectionUnitList->insertRow(i);
        ui->collectionUnitList->setItem(i,0,new QTableWidgetItem(curUnit));
        if(actives[i] == "BOTH")
           // ui->collectionUnitList->item(i,0)->setBackground(Qt::yellow);
            yellowBack(ui->collectionUnitList->item(i,0));
        else if(actives[i] == "ACTIVE")
           greenBack(ui->collectionUnitList->item(i,0));
            //ui->collectionUnitList->item(i,0)->setBackground(Qt::green);
        else
            redBack(ui->collectionUnitList->item(i,0));
            //ui->collectionUnitList->item(i,0)->setBackground(Qt::red);


    }

}

QStringList MainWindow::setActiveInactive(QStringList units, QList<miniCard> cards){
    QStringList activeList;
    int active, inactive;
    int curUnit;
    for(int i = 0; i < units.length(); i++){
        active = 0;
        inactive = 0;
        curUnit = units[i].toInt();
        for(int j = 0; j < cards.length(); j++){
            if(cards[j].unit > curUnit){
                break;
            }
            if(cards[j].unit == curUnit){
                if(cards[j].active)
                    active++;
                else
                    inactive++;
            }
        }
        if(active > 0 && inactive > 0){
            activeList.push_back("BOTH");
        }
        else if(active > 0 && inactive == 0){
            activeList.push_back("ACTIVE");
        }
        else{
            activeList.push_back("INACTIVE");
        }

    }
    return activeList;
}
QString MainWindow::greenString(QString cur){
    QString ret = "";
    ret += "<font color='green'>";
    ret += cur;
    ret += "</font>";
    return ret;
}

QString MainWindow::yellowString(QString cur){
    QString ret = "";
    ret += "<font color='yellow'>";
    ret += cur;
    ret += "</font>";
    return ret;
}


QString MainWindow::redString(QString cur){
    QString ret = "";
    ret += "<font color='red'>";
    ret += cur;
    ret += "</font>";
    return ret;
}
void MainWindow::yellowBack(QTableWidgetItem * widget){
    widget->setBackground(Qt::yellow);
}
void MainWindow::greenBack(QTableWidgetItem * widget){
    widget->setBackground(Qt::green);
}

void MainWindow::redBack(QTableWidgetItem * widget){
    widget->setBackground(Qt::red);
}
void MainWindow::randomBack(QTableWidgetItem *widge, int index){
     widge->setBackground(bgColours[index]);
}


void MainWindow::on_collectionUnitList_clicked(const QModelIndex &index)
{
    int row = ui->collectionUnitList->currentRow();
    int selectedRowUnit = ui->collectionUnitList->item(row,0)->text().toInt();
    if(selectedRowUnit == lastRowSelected)
        return;
    lastRowSelected = selectedRowUnit;

    populateCardList(selectedRowUnit);


}
void MainWindow::populateCardList(int index){

    lastRowSelected = -1;
    ui->collectionCardList->clear();
    ui->collectionCardList->setRowCount(0);
    ui->collectionCardList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->collectionCardList->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->collectionCardList->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->collectionCardList->setColumnCount(1);
    ui->collectionCardList->horizontalHeader()->setStretchLastSection(true);
    ui->collectionCardList->verticalHeader()->hide();
    QStringList headers;
    headers.push_back("Card");
    ui->collectionCardList->setHorizontalHeaderLabels(headers);
    int curIndex = 0;
    for(int i = 0; i < miniList.length(); i++){
        if(miniList[i].unit > index)
            return;
        if(miniList[i].unit != index)
            continue;

        ui->collectionCardList->insertRow(curIndex);
        ui->collectionCardList->setItem(curIndex,0,new QTableWidgetItem(miniList[i].front));
        if(miniList[i].active){
            greenBack(ui->collectionCardList->item(curIndex,0));
        }
        else{
            redBack(ui->collectionCardList->item(curIndex,0));
        }
        curIndex++;
    }
}



void MainWindow::displayMessage(QString message){
    QMessageBox msg;
    msg.setText(message);
    msg.exec();
}
void threadStarter(){
    pthread_t clock;
    pthread_create(&clock,NULL,clockThread, (void *)0);
}
void * clockThread(void * args){

    c->startClock();
}

void MainWindow::on_studyAnswerEdit_textEdited(const QString &arg1)
{
    c->keyPushed();
}
QString MainWindow::addHintToHint(QString hint, QString cardHint, QString answer){
    if(cardHint.length()==0)
        return hint;
    QString ret = hint;
    for(int i = 0; i < cardHint.length(); i++){
        for(int j = 0; j < hint.length(); j++){
            if(cardHint[i] == answer[j])
                ret[j] = cardHint[i];
        }
    }
    return ret;
}
/*
void MainWindow::on_studyMainMenu_clicked()
{
    study.clean();
    countdownClock->stopClock();
    c->stopClock();
    if(multiStudy)
        networkedManager.sendExit();
    returnToMain();
}
*/


void MainWindow::on_collectionActivate_clicked()
{
    int row = ui->collectionUnitList->currentRow();
    if(row < 0)
        return;
    int val = ui->collectionUnitList->item(row,0)->text().toInt();
    db.setActive(curDeck, val);
    updateMiniList(val, true);
    updateUnitList(row, true);
    populateCardList(row);
}

void MainWindow::on_collectionDeactivate_clicked()
{
    int row = ui->collectionUnitList->currentRow();
    if(row < 0)
        return;
    int val = ui->collectionUnitList->item(row,0)->text().toInt();
    db.setInactive(curDeck, val);
    updateMiniList(val, false);
    updateUnitList(row, false);
    populateCardList(row);

}
void MainWindow::updateMiniList(int unit, bool act){
    for(int i =0 ; i < miniList.length(); i++){
        if(miniList[i].unit == unit)
            miniList[i].active = act;
    }
}
void MainWindow::updateUnitList(int unit, bool act){
    if(act)
        greenBack(ui->collectionUnitList->item(unit,0));
    else
        redBack(ui->collectionUnitList->item(unit,0));
}

void MainWindow::on_mainStatsButton_clicked()
{

    Graph * grapher = new Graph();
    grapher->setData(&db);
    grapher->exec();
}



void MainWindow::on_mainConnect_clicked()
{
    int fd;
    fd = connectTCPSocket(serverPort.toInt(),(char *)serverIp.toStdString().c_str());
    if(fd == -1){
        displayMessage("Error connecting to server");
        return;
    }
    networkedManager.setConnected(fd);
    ui->mainConnect->setEnabled(false);
    pthread_t networkThread;
    connected = true;
    registerSocket(fd, 15);
    pthread_create(&networkThread, 0, networkWorker, (void *)0);
    ui->mainMulti->setEnabled(true);
    ui->mainLogin->setEnabled(true);

}

void MainWindow::on_deckManagerExport_clicked()
{
    setPage(EXPORT);
}

void MainWindow::on_exportExportButton_clicked()
{
    QString description, deck, language;
    description = ui->exportDescription->text();
    language = ui->exportLanguage->text();
    if(description.length() == 0){
        displayMessage("Please enter a description");
        return;
    }
    if(language.length() == 0){
        displayMessage("Please enter a language");
        return;
    }
    deck = ui->deckTitle->text();
    networkedManager.doExport(deck,description,language);

}

void MainWindow::on_deckManagerImportFromServer_clicked()
{
    setPage(IMPORT);
    networkedManager.requestDeckList();
}
void MainWindow::updateImportList(QStringList list){
    ui->importTable->clear();
    ui->importTable->setRowCount(0);
    ui->importTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->importTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->importTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->importTable->setColumnCount(2);
    ui->importTable->horizontalHeader()->setStretchLastSection(true);
    ui->importTable->verticalHeader()->hide();
    QStringList headers;
    headers.push_back("DeckID");
    headers.push_back("Cards");
    ui->importTable->setHorizontalHeaderLabels(headers);
    for(int i = 0; i < list.length(); i++){
        QStringList parts = list[i].split('\t');
        if(parts.length() < 6)
            continue;
        ui->importTable->insertRow(i);
        importDeckData dat;
        dat.creator = parts[2];
        dat.deckID = parts[0];
        dat.desc = parts[3];
        dat.language = parts[4];
        dat.numCards = parts[5];
        importData.push_back(dat);



        ui->importTable->setItem(i,0,new QTableWidgetItem(parts[1]));
        ui->importTable->setItem(i,1,new QTableWidgetItem(parts[5]));

    }
}

void MainWindow::on_importGetDeck_clicked()
{
    int row = ui->importTable->selectionModel()->currentIndex().row();
    if(row == -1){
        return;
    }
    QString deckID;
    QString deckName = ui->importTable->item(row,0)->text();
    if(row > importData.length())
        displayMessage("Error, deck not found");
    deckID = importData[row].deckID;
    QString check = db.addImportDeck(deckID, deckName);
    if(check.length() == 0){
        displayMessage("Error creating deck");
        return;
    }
    networkedManager.sendImportRequest(deckID, check);
}

void MainWindow::on_multiStudyButton_clicked()
{
    int row = ui->multiLobbyTable->selectionModel()->currentIndex().row();
    if(row == -1){
        return;
    }
    networkedManager.sendMultiJoinRequest(curDeck, row);
    setupScoreTable();

}

void MainWindow::on_mainMulti_clicked()
{
    int row = ui->mainDeckView->selectionModel()->currentIndex().row();
    if(row == -1){
        return;
    }
    QString deckName = ui->mainDeckView->item(row, 0)->text();
    curDeck = deckName;
       // this->resize(1260, 750);
    setPage(MULTIPAGE);
   // this->resize(1260, 750);
    ui->multiLobbyTable->clear();
    ui->multiLobbyTable->setRowCount(0);
    networkedManager.sendMultiRequest(deckName);
}

void MainWindow::on_multiCreateRoom_clicked()
{

    int games = 0;
    int ind = 0;
    if(ui->multiIndiCheck->isChecked())
        ind = 1;
    else{
        pthread_t clock;
        pthread_create(&clock, 0, cdThread, (void *)0);
    }
    if(ui->multiGamesCheck->isChecked())
        games = 1;

    int interval = ui->multiRoundSlider->value();
    networkedManager.createRoom(curDeck, interval, games,ind);
    setupScoreTable();
    ui->studyAnswerButton->setEnabled(true);
    answerLock = false;
}
void MainWindow::updateRoomList(){
    QList<roomData>rooms = networkedManager.availableRooms;
    ui->multiLobbyTable->clear();
    ui->multiLobbyTable->setRowCount(0);
    ui->multiLobbyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->multiLobbyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->multiLobbyTable->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->multiLobbyTable->setColumnCount(4);
    ui->multiLobbyTable->horizontalHeader()->setStretchLastSection(true);
    ui->multiLobbyTable->verticalHeader()->hide();
    QStringList headers;
    headers.push_back("Cards");
    headers.push_back("Users");
    headers.push_back("Interval");
    headers.push_back("Group");
    int rows = 0;
    ui->multiLobbyTable->setHorizontalHeaderLabels(headers);
    for(int i = 0; i < rooms.length(); i++){
        ui->multiLobbyTable->insertRow(rows++);
        ui->multiLobbyTable->setItem(i,0,new QTableWidgetItem(rooms[i].matches));
        ui->multiLobbyTable->setItem(i,1,new QTableWidgetItem(rooms[i].members));
        ui->multiLobbyTable->setItem(i,2,new QTableWidgetItem(rooms[i].interval));
        if(rooms[i].independant)
            ui->multiLobbyTable->setItem(i,3,new QTableWidgetItem("No"));
        else
            ui->multiLobbyTable->setItem(i,3,new QTableWidgetItem("Yes"));

    }
}
void MainWindow::joinMultiStudy(){
    doBasicStudySetup();

    if(networkedManager.isIndependant()){
        study.loadMultiCardList(db, networkedManager.getCardList(), true);
        multiLoadQuestion(-1);
    }
    else{
        pthread_t clock;
        pthread_create(&clock, 0, cdThread, (void *)0);
        study.loadMultiCardList(db, networkedManager.getCardList(), false);
        multiLoadQuestion(-2);
    }
}
void MainWindow::roomFull(){
    displayMessage("Room full, try another");
}
void MainWindow::roomGone(){
    displayMessage("Room no longer exists, try another");
}
void MainWindow::newMultiCreated(){
    doBasicStudySetup();
    if(networkedManager.isIndependant()){
        study.loadMultiCardList(db, networkedManager.getCardList(), true);
    }
    else{
        study.loadMultiCardList(db, networkedManager.getCardList(), false);
    }
    QList<int> card = networkedManager.getCardList();
    if(card.length() > 0)
        multiLoadQuestion(card[0]);
    else
        multiLoadQuestion(-2);



}
void MainWindow::doBasicStudySetup(){
    study.setDeck(curDeck);
    if(!study.setSession(db))
        return;

    study.loadGrammar(db);
    int interval = db.getInterval(curDeck);
    study.setMaxInterval(interval);
    multiStudy = true;
    QSqlQuery result = db.getTodaysStats();
    setTodayStat(result);
    setPage(STUDY);
    ui->studyMultiBox->setHidden(false);
    threadStarter();
}

void MainWindow::disconnectFromMulti(){
    networkedManager.closeConnect();
}
void MainWindow::clearAllStudy(){
    ui->studyAnswerEdit->setText("");
    ui->studyAnswerLabel->setText("");
    ui->studyAnswerPercentLabel->setText("");
    ui->studyFront->setText("");
    ui->studyHintEdit->setText("");
    ui->studyNewAverage->setText("");
    ui->studyNewInterval->setText("");
    ui->studyPastAttempts->setText("");
    ui->studyPastAverage->setText("");
    ui->studyPastInterval->setText("");
}
void MainWindow::serverDied(){
    if(multiStudy){
        returnToMain();
        study.clean();
    }
    connected = false;
    ui->mainConnect->setEnabled(true);
    ui->mainMulti->setEnabled(false);
    ui->mainLogin->setEnabled(false);
    ui->friendButton->setEnabled(false);
}
void MainWindow::updateScores(){
    ui->studyScoreList->clear();
    ui->studyScoreList->setRowCount(0);
    ui->studyScoreList->setColumnCount(2);
    QStringList headers;
    headers.push_back("User");
    headers.push_back("Score");
    ui->studyScoreList->setHorizontalHeaderLabels(headers);
    QList<users> userList = networkedManager.userList;
    int row = 0;
    if(userList.length() == 0)
        return;
    do{
        int max = userList[0].score - userList[0].startScore;
        int ind = 0;

        for(int i = 0; i < userList.length(); i++){
            int realScore = userList[i].score - userList[i].startScore;
            if(realScore > max){
                max = realScore;
                ind = i;
            }
        }
        ui->studyScoreList->insertRow(row);
        QString name = userList[ind].name;
        ui->studyScoreList->setItem(row,0,new QTableWidgetItem(name));
        ui->studyScoreList->setItem(row,1,new QTableWidgetItem(QString::number(max)));
        int realIndex = -1;
        for(int j = 0; j < networkedManager.userList.length(); j++){
            if(name == networkedManager.userList[j].name){
                realIndex = j;
                break;
            }
        }
        if(realIndex != -1 && realIndex < 10){
            randomBack(ui->studyScoreList->item(row,0), realIndex);

        }
        userList.removeAt(ind);

    }while(userList.length() > 0);

}
void MainWindow::setupScoreTable(){
    ui->studyScoreList->clear();
    ui->studyScoreList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //ui->studyScoreList->setSelectionBehavior(QAbstractItemView::NoSelection);
    ui->studyScoreList->setColumnCount(2);
    ui->studyScoreList->horizontalHeader()->setStretchLastSection(true);

    ui->studyScoreList->verticalHeader()->hide();
    QStringList headers;
    headers.push_back("User");
    headers.push_back("Score");
    ui->studyScoreList->setHorizontalHeaderLabels(headers);
    ui->studyScoreList->setColumnWidth(0, ui->studyScoreList->width()/10*7);;

}

void MainWindow::on_studySendChat_returnPressed()
{
    QString message = ui->studySendChat->text();
    networkedManager.sendChat(message);
    ui->studySendChat->setText("");
}
void MainWindow::addChat(QString message){
    QStringList parts = message.split('>');
    int realIndex = -1;
    if(parts.length() > 1){
        QString name = parts[0];
        for(int i = 0; i < networkedManager.userList.length(); i++){
            if(name == networkedManager.userList[i].name){
                realIndex = i;
                break;
            }
        }
    }
    if(realIndex > -1 && realIndex < 10){
        ui->studyChat->moveCursor( QTextCursor::End );
        QTextCursor cursor( ui->studyChat->textCursor() );

        QTextCharFormat format;
        format.setFontWeight( QFont::DemiBold );
        format.setForeground( QBrush( bgColours[realIndex] ) );
        cursor.setCharFormat( format );

        cursor.insertText( message );
        //cursor.insertText('\n');
        ui->studyChat->append("");


    }
 //   ui->studyChat->append(message);
}
void * cdThread(void *args){
    countdownClock->setDown();
    countdownClock->startClock();
}


void MainWindow::gameClickLogic(int guess, bool clicked){
    clicked = !clicked;
    if(guessMade){
        //second button clicked - evaluate if 2 are seperate

        //This was clicked previously - so cancel it out
        if(clicked){
            guessMade = false;
            gameFirstGuess = -1;
            gameButtons[guess]->setChecked(false);
        }else{
            //Second guess, so send for eval
            gameSecondGuess = guess;
            networkedManager.sendGameGuess(gameFirstGuess, gameSecondGuess);
            gameButtons[gameFirstGuess]->setChecked(false);
            gameButtons[gameSecondGuess]->setChecked(false);
            guessMade = false;
            gameFirstGuess = -1;
            gameFirstGuess = -1;
        }

    }else{
        guessMade = true;
        gameFirstGuess = guess;
        //first button clicked
    }
}
void MainWindow::updateGameBoard(int index, int first, int second){
    isCurrentlyChecked(first);
    isCurrentlyChecked(second);
    displayGameAnswer(first,second,index);


}
bool MainWindow::isCurrentlyChecked(int place){
    if(gameButtons[place]->isChecked()){
        guessMade = false;
        gameButtons[place]->setChecked(false);
        gameFirstGuess = -1;
        return true;
    }
    return false;
}
void MainWindow::displayGameAnswer(int first, int second, int index){
    gameButtons[first]->setEnabled(false);
    QString qss = QString("background-color: %1").arg(bgColours[index].name());
    gameButtons[first]->setStyleSheet(qss);

    gameButtons[second]->setEnabled(false);
    qss = QString("background-color: %1").arg(bgColours[index].name());
    gameButtons[second]->setStyleSheet(qss);

}


void MainWindow::loadGame(){
    setPage(GAME);
    for(int i = 0; i < 16; i++){
        gameButtons[i]->setStyleSheet("");
        gameButtons[i]->setEnabled(true);
        if(i < networkedManager.gameList.length())
            gameButtons[i]->setText(networkedManager.gameList[i]);
    }
}


void MainWindow::on_game0_clicked()
{

    gameClickLogic(0, ui->game0->isChecked());
}

void MainWindow::on_game1_clicked()
{
    gameClickLogic(1, ui->game1->isChecked());
}

void MainWindow::on_game2_clicked()
{
    gameClickLogic(2, ui->game2->isChecked());
}

void MainWindow::on_game3_clicked()
{
    gameClickLogic(3, ui->game3->isChecked());
}

void MainWindow::on_game4_clicked()
{
    gameClickLogic(4, ui->game4->isChecked());
}

void MainWindow::on_game5_clicked()
{
    gameClickLogic(5, ui->game5->isChecked());
}

void MainWindow::on_game6_clicked()
{
    gameClickLogic(6, ui->game6->isChecked());
}

void MainWindow::on_game7_clicked()
{
    gameClickLogic(7, ui->game7->isChecked());
}
void MainWindow::on_game8_clicked()
{
    gameClickLogic(8, ui->game8->isChecked());
}
void MainWindow::on_game9_clicked()
{
    gameClickLogic(9, ui->game9->isChecked());
}

void MainWindow::on_game10_clicked()
{
    gameClickLogic(10, ui->game10->isChecked());
}

void MainWindow::on_game11_clicked()
{
    gameClickLogic(11, ui->game11->isChecked());
}

void MainWindow::on_game12_clicked()
{
    gameClickLogic(12, ui->game12->isChecked());
}

void MainWindow::on_game13_clicked()
{
    gameClickLogic(13, ui->game13->isChecked());
}

void MainWindow::on_game14_clicked()
{
    gameClickLogic(14, ui->game14->isChecked());
}

void MainWindow::on_game15_clicked()
{
    gameClickLogic(15, ui->game15->isChecked());
}
void MainWindow::loadGameButtons(){
    gameButtons[0] = ui->game0;
    gameButtons[1] = ui->game1;
    gameButtons[2] = ui->game2;
    gameButtons[3] = ui->game3;
    gameButtons[4] = ui->game4;
    gameButtons[5] = ui->game5;
    gameButtons[6] = ui->game6;
    gameButtons[7] = ui->game7;
    gameButtons[8] = ui->game8;
    gameButtons[9] = ui->game9;
    gameButtons[10] = ui->game10;
    gameButtons[11] = ui->game11;
    gameButtons[12] = ui->game12;
    gameButtons[13] = ui->game13;
    gameButtons[14] = ui->game14;
    gameButtons[15] = ui->game15;


}
void MainWindow::returnToStudy(){
    setPage(STUDY);
}

void MainWindow::on_mainLogin_clicked()
{
    if(menu == 0)
        menu = new LoginMenu(this, this, &networkedManager);
    if(friendMenu == 0)
        friendMenu = new Friends(this, this, &networkedManager);
    menu->exec();
}
void MainWindow::passToLogin(int code){
    QMetaObject::invokeMethod(menu,"handleLogin", Q_ARG(int, code));
    if(code == 2){
        ui->friendButton->setEnabled(true);
        ui->mainLogin->setEnabled(false);
        friendMenu->setOurName(loggedInName);
        ui->statusBar->showMessage("Logged in as " + loggedInName);
    }
}
void MainWindow::passToRegister(int code){
    QMetaObject::invokeMethod(menu,"handleRegister", Q_ARG(int, code));
}
void MainWindow::passFriendAdd(int code){
    if(friendMenu == 0)
        friendMenu = new Friends(this,this,&networkedManager);

    QMetaObject::invokeMethod(friendMenu,"handleAdd", Q_ARG(int, code));
}

void MainWindow::on_friendButton_clicked()
{
    if(friendMenu == 0)
        friendMenu = new Friends(this,this,&networkedManager);
    QString qss = QString("");
    ui->friendButton->setStyleSheet(qss);    friendMenu->exec();
}
void MainWindow::passFriendUpdate(QString online, QString offline){
    if(friendMenu == 0)
        friendMenu = new Friends(this,this,&networkedManager);

    QMetaObject::invokeMethod(friendMenu,"handleUpdate", Q_ARG(QString, online), Q_ARG(QString, offline));
}
void MainWindow::passFriendRemove(int code){
    if(friendMenu == 0)
        friendMenu = new Friends(this,this,&networkedManager);

    QMetaObject::invokeMethod(friendMenu,"handleFriendRemove", Q_ARG(int, code));
}
void MainWindow::passPm(QString friendName, QString message){
    if(friendMenu == 0)
        friendMenu = new Friends(this,this,&networkedManager);

    if(!friendMenu->isVisible()){
        QMetaObject::invokeMethod(this,"newPm");


    }
    QMetaObject::invokeMethod(friendMenu,"handlePm", Q_ARG(QString, friendName), Q_ARG(QString, message));
}
void MainWindow::newPm(){
    QString yellow = "* { background-color: rgb(255,255,0) }";

    ui->friendButton->setStyleSheet(yellow);
}

void MainWindow::passSentPm(QString friendName, QString message){
    if(friendMenu == 0)
        friendMenu = new Friends(this,this,&networkedManager);

    QMetaObject::invokeMethod(friendMenu,"handleSentPm", Q_ARG(QString, friendName), Q_ARG(QString, message));
}
void MainWindow::passUserName(){
    if(friendMenu == 0)
        friendMenu = new Friends(this,this,&networkedManager);

    friendMenu->setOurName(loggedInName);
}

void MainWindow::on_mainMainReturn_clicked()
{
    //ui->stackedWidget->setCurrentIndex(MAINPAGE);
    int current = ui->stackedWidget->currentIndex();
    if(current == STUDY){
        study.clean();
        countdownClock->stopClock();
        c->stopClock();
        if(multiStudy)
            networkedManager.sendExit();
    }


    returnToMain();
}
void MainWindow::loadConfig(){
    QFile f(CONFIGFILE);

    if(!f.open(QIODevice::ReadOnly)){
        qDebug() << "Error loading config file";
        serverPort = SERVERPORT;
        serverIp = SERVERIP;
        return;
    }
    QString line = f.readLine();
    line = line.left(line.length()-1);
    serverIp = line;
    serverIp += '\0';
    line = f.readLine();
    line = line.left(line.length()-1);
    serverPort = line;

    f.close();
}
