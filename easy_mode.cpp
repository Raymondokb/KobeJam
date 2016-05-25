#include "easy_mode.h" // for easy_mode function declarations in header
#include "ui_easy_mode.h"
#include <QPainter> // for QPainter
#include <QDebug> // for qDebug()/debugging purposes
#include <QPoint> // for setX/setY functions
#include <QSize> // for our size variables (i.e. how big/long our baller and hoop and ball is)
#include <QTimer> // for animating the hoop (and the ball maybe?)
#include <QRect>
#include <string>
#include <sstream>


/** easy_mode constructor
 *
 * Brief: sets up the ui and also initializes the timer, sizes, locations and images for the scene
 *
 * @param parent: If a QWidget pointer is included as parameter, then that pointer acts as a parent
 * to the easy_mode object.
 */
easy_mode::easy_mode(QWidget* parent) : QWidget(parent), ui(new Ui::easy_mode)
{
    ui->setupUi(this);

    //initialize images...
    background_image = new QPixmap(":/images/elliot bball game court 1 copy (small).jpg"); // background
    baller_image = new QPixmap(":/images/elliot-bball-game-shooter-1.png");
    hoop_image = new QPixmap(":/images/elliot-bball-game-hoop-1.png");
    ball_image = new QPixmap(":/images/elliot-bball-game-ball-1.png");


    //first we grab our screen dimensions
    screen_width = this->width();
    screen_height = this->height();

    //initialize baller_size and baller_position
    baller_size = new QSize(screen_width/12, screen_height/4); //now we have the size
    baller_position = new QPoint(screen_width/2, screen_height - baller_size->height());

    // initialize hoop_size and hoop_position
    hoop_size = new QSize(screen_width/6, screen_height/9);
    hoop_position = new QPoint((screen_width - hoop_size->width())/2, 0);

    // let's also link up a timer with our hoop
    hoop_and_ball_timer = new QTimer(this); // might need to delete this stuff....
    connect(hoop_and_ball_timer, SIGNAL(timeout()), this, SLOT(animate_hoop())); // but don't start yet...

    // let's set up the ball variables...
    ball_size = new QSize(screen_width/12, screen_width/12);
    ball_default_height = baller_position->y() - ball_size->height();
    ball_position = new QPoint(baller_position->x(), ball_default_height); // want to match baller position

    // also connect our ball and our check time remaining function to the timer
    connect(hoop_and_ball_timer, SIGNAL(timeout()), this, SLOT(animate_ball()));
    hoop_and_ball_timer->start(10);

    // we need a separate timer in order to
    shot_clock_per_second = new QTimer(this);
    connect(shot_clock_per_second, SIGNAL(timeout()), this, SLOT(display_time_remaining()));

    // now let's create a new timer that signals the end of the game...
    shot_clock = new QTimer(this);
    connect(shot_clock, SIGNAL(timeout()), parent, SLOT(start_win_screen()));
    shot_clock->setSingleShot(true);
    shot_clock->start(60000); // should last us 90 seconds/1 min 30 seconds

    // also start ou shot_clock_per_second timer so taht we can display the remaining time accurately
    shot_clock_per_second->start(1000); // triggers every one second
}


/** display_time_remaining
 *
 * Brief: updates the QLabel in the top left corner to match the time remaining
 *
 * @param void
 * @return void
 */
void easy_mode::display_time_remaining()
{
    --time_remaining;

    // need to convert our time_remaining to a string, then a QString
    std::string time_string;
    std::ostringstream temp;
    temp << time_remaining;
    time_string = temp.str();

    QString qs = QString::fromStdString(time_string);
    ui->label_2->setText(qs);
}

/** easy_mode destructor
 *
 * Brief: deletes all objects not given to the easy_mode pointer
 *
 */
easy_mode::~easy_mode()
{
    // Let's clean house...
    delete background_image;
    delete hoop_and_ball_timer;
    delete shot_clock;
    delete shot_clock_per_second;

    delete baller_position;
    delete baller_size;
    delete baller_image;

    delete hoop_position;
    delete hoop_size;
    delete hoop_image;

    delete ball_position;
    delete ball_size;
    delete ball_image;

    delete ui;
}

/** paint event function
 *
 * Brief: paints and repaints the ball, the background, the hoop and the player to the screen
 *
 * @param e: a QPaintEvent pointer, which we do not use
 * @return void
 */
void easy_mode::paintEvent(QPaintEvent *e)
{
    QPainter fancy_french_painter(this);

    // background image!
    fancy_french_painter.drawPixmap(0,0,this->width(),this->height(), *background_image);

    // DRAW THE KOBE!
    fancy_french_painter.drawPixmap(baller_position->x(), baller_position->y(), baller_size->width(), baller_size->height(), *baller_image);

    // DRAW THE HOOP
    fancy_french_painter.drawPixmap(hoop_position->x(), hoop_position->y(), hoop_size->width(), hoop_size->height(), *hoop_image);

    // DRAW THE BALL
    fancy_french_painter.drawPixmap(ball_position->x(), ball_position->y(), ball_size->width(), ball_size->height(), *ball_image);
}

/** key press event function
 *
 * Brief: allows the user to send keyboard input and thus shoot the ball and move the player character
 *
 * @param event: a QKeyEvent pointer, which we do not use
 * @return void
 */
void easy_mode::keyPressEvent(QKeyEvent* event)
{
    switch(event->key())
    {
        case Qt::Key_Left: // if the key press is left, then...
            if(baller_position->x() > 0) // move the baller's location left
            {
                baller_position->setX(baller_position->x() - x_change_per_press);

                // we also want to change ball position if it was not shot yet
                if(ball_in_motion == false)
                    ball_position->setX(ball_position->x() - x_change_per_press);
            }
            break;
        case Qt::Key_Right: // if the key press is right, then...
            if(baller_position->x() < (screen_width - baller_size->width())) // move the baller's location right
            {
                baller_position->setX(baller_position->x() + x_change_per_press);

                // we also want to change ball position if it was not shot yet
                if(ball_in_motion == false)
                    ball_position->setX(ball_position->x() + x_change_per_press);
            }
            break;
        case Qt::Key_Space:
            // if the ball has been reset...
            if(ball_position->y() == ball_default_height)
            {
                ball_in_motion = true;
            }
            break;
    }

    //repaint the screen with updated positions
    this->repaint();
}


/** show event function
 *
 * Brief: gives the window focus and thus allows for keyboard input to work
 *
 * @param event: a QShowEvent pointer, which we do not use
 * @return void
 */
void easy_mode::showEvent(QShowEvent *event)
{
    //widget activated
    this->activateWindow();

    //now the easy_mode widget/object gets focus
    this->setFocus();

    //not sure what this does...
    QWidget::showEvent(event);
}


/** is ball in hoop function
 *
 * Brief: checks to see if the current game state has the ball and the hoop colliding
 *
 * @param void
 * @return If the ball and the hoop are making contact at the time of the function's call, then return true, else falase
 */
bool easy_mode::is_ball_in_hoop()
{
    // swish?
    bool swish;

    // let's grab the ball's current center x coordinate
    size_t ball_x_center = ball_position->x() + (ball_size->width())/2;

    // now check to see if the ball swished through the hoop!
    if(ball_x_center > hoop_position->x() && ball_x_center < (hoop_position->x() + hoop_size->width()))
    {
        swish = true;
        this->points += 2;

        // now change the text on screen
        display_points();
    }
    else
    {
        swish = false;
    }

    return swish;
}


/** display points function
 *
 * Brief: updates the points on the top right hand corner
 *
 * @param void
 * @return void
 */
void easy_mode::display_points()
{
    // now convert our score to a string, then a QString
    std::string points_string;
    std::ostringstream temp2;
    temp2 << points;
    points_string = temp2.str();

    QString qs2 = QString::fromStdString(points_string);
    ui->label_4->setText(qs2);
}


/** animate hoop function
 *
 * Brief: animatse the hoop left to right
 *
 * @param void
 * @return void
 */
void easy_mode::animate_hoop()
{
    // if the hoop is too far to the right...
    if((hoop_position->x() + hoop_size->width()) >= screen_width)
    {
        hoop_change_per_timeout *= -1; // we reverse the change
    }
    // if the hoop is too far to the left...
    else if(hoop_position->x() <= 0)
    {
        hoop_change_per_timeout *= -1;
    }

    //now add the hoop_change_per_timeout to the position
    hoop_position->setX(hoop_position->x() + hoop_change_per_timeout);

    //call for a repaint()
    this->repaint();
}


/** animate ball function
 *
 * Brief: animates the ball after the player presses the space bar
 *
 * @param void
 * @return void
 */
void easy_mode::animate_ball()
{
    // so if the ball passes the hoop's bottom side
    if(ball_position->y() <= hoop_size->height())
    {
        // this will increment our score or not depending on if the ball was in the hoop!
        is_ball_in_hoop();

        // reset the ball
        ball_position->setX(baller_position->x());
        ball_position->setY(ball_default_height);

        // set ball_in_motion as false
        ball_in_motion = false;
    }

    if(ball_in_motion == true)
    {
       ball_position->setY(ball_position->y() - ball_change_per_timeout);
    }

    this->repaint();
}

