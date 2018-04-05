#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "cardlist.h"
StudyClock * c;
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
}

void MainWindow::on_mainDeckManager_clicked()
{
    int row = ui->mainDeckView->selectionModel()->currentIndex().row();
    if(row == -1){
        return;
    }
    QString deckName = ui->mainDeckView->item(row, 0)->text();
    deckM.loadDeck(deckName);
    ui->deckManagerTitle->setText(deckName);
    curDeck = deckName;
    setPage(DECKMANAGER);
}
void MainWindow::setPage(int i){
    ui->stackedWidget->setCurrentIndex(i);

}

void MainWindow::on_deckManagerAddCard_clicked()
{
    ui->addCardTitle->setText(deckM.getDeck());
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

    Graph * grapher = new Graph();
    grapher->setData(&db);
    grapher->exec();
    return;
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
    ui->deckSumTitle->setText(deck);
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
    QString deckName = ui->deckSumTitle->text();
    int interval = db.getInterval(deckName);
    study.setMaxInterval(interval);

    QSqlQuery result = db.getTodaysStats();
    setTodayStat(result);

    setPage(STUDY);
    if(study.isSentences())
        ui->studySentenceCheck->setChecked(true);
    else
        ui->studySentenceCheck->setChecked(false);

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
        QString answer = ui->studyAnswerEdit->text();
        if(answer.length() == 0)
            return;
        answerString = study.evalAnswer(*curCard,answer, ui->studySentenceCheck->isChecked(), &answerVal, ui->studyHintLabel->text());

        dealWithAnswer(answer, answerVal, answerString);

        question = false;

    }
    else{
        singleStudyLoadQuestion();
    }



}
void MainWindow::singleStudyLoadQuestion(){
    if(!study.getNext(&curCard)){
        study.clean();
        returnToMain();
        return;
    }
    loadQuestion(curCard);

}
void MainWindow::addSecToClock(){
    studyTime = studyTime.addSecs(1);
    ui->studyElapsedTIme->setText("Time Studied: " + studyTime.toString(Qt::ISODate));
}

void MainWindow::loadQuestion(Card * c){
    hideStudyAnswer();
    ui->studyFront->setText(c->front);
    ui->studyHintEdit->setText(c->hint);

    ui->studyFront->setText(curCard->front);
    if(ui->studyHintCheck->isChecked()){
        QString hint = study.loadHint(*curCard, ui->studySentenceCheck->isChecked());

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

    updateScoringDue(ansVal);

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

void MainWindow::updateScoringDue(int ans){
    int newInterval;
    int newAverage;
    int newLevel;
    int newAttempts;
    newLevel = -1;
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
        }
        else if(!curCard->newCard){
            correct++;
            study.remove();
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
    QString deck = ui->deckManagerTitle->text();
    ui->collectionDeckTitle->setText(deck);
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
    qDebug() << "Complete loading";

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

void MainWindow::on_importReturnMain_clicked()
{
    returnToMain();
}

void MainWindow::on_createReturnMain_clicked()
{
    returnToMain();

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

void MainWindow::on_studyMainMenu_clicked()
{
    study.clean();
    returnToMain();
}


void MainWindow::on_collectionActivate_clicked()
{
    int row = ui->collectionUnitList->currentRow();
    if(row < 0)
        return;
    int val = ui->collectionUnitList->item(row,0)->text().toInt();
    db.setActive(curDeck, val);
    updateMiniList(val, true);
    updateUnitList(val, true);
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
    updateUnitList(val, false);
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


