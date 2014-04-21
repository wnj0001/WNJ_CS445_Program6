/*********************************************************************

    Software Architecture Statement:

    This program ...
    
    WNJ  04/2014

 ********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif
#include "my_setup_3D.h"

//  Constants for use with the my_setup() function.
#define canvas_width 400
#define canvas_height 600
#define canvas_name "Blaster Game"

// Represents a point in 3-Dimensional space.
typedef struct {
    float x;
    float y;
    float z;
} Point;

// Represents a rgb-style color.
typedef struct {
    float red;
    float green;
    float blue;
} Color;

// Represents a 3-Dimensional cube object and its attributes.
//      Movement is tri-state:
//          0: no movement
//          1: movement to left
//          2: movement to right
typedef struct {
    Point center;
    float size;
    Color color;
    int is_alive;
    int movement;
} Cube;

// Represents a quadrilateral object and its attributes
typedef struct {
    Point vertices[4];
    Point center;
    Point translation;
    float rotation_angle;
    int rotation_axis;
    int rotation_step;
} Quad;

// Pointer to Point object that represents the origin of the scene.
Point* origin;

// Float that represents the current location of the z-plane on which 
// the drawn objects' origins lie.
float z_plane;

// Float that represents the current frame rate of the animation
float frame_rate;

// Floats that represent the size of the player and enemy cubes
float player_size;
float enemy_size;

// Pointers to Point objects that represent the starting positions
// of the player and enemy ships
Point* player_start;
Point* enemy_start;

// Pointers to Color objects that represent the colors used in the 
// program.
//      - White for the background, 
//      - player_color for player lines,
//      - Red for the enemy ships.
Color* bg_color;
Color* player_color;
Color* enemy_color;
Color* corner_color;

// Pointers to Cube objects representing the player and enemy ships.
Cube* player;
Cube* enemy;

// Integers used to calculate the spawn point for the enemy ships.
int enemy_min_x, enemy_max_x;
int enemy_spawn_x;

// Integers used to calculate the time interval at which the enemy
// ships spawn.
int enemy_min_time, enemy_max_time;
int enemy_spawn_time;

// Floats used to calculate the rate at which the enemy ships move
// down the glSwapBuffers. 
float enemy_total_dist;
float enemy_total_time;
float enemy_step_dist;

// Floats used to calculate the rate at which the player's ship 
// can move along the bottom of the canvas.
float player_total_dist;
float player_total_time;
float player_step_dist;

// A boolean integer used to determine if the laser is currently
// firing.
int is_laser_firing;

// An integer representing the player's total score.
int player_score;

// A boolean integer used to determine if the game has entered the
// game over state.
int is_game_over;

// Quad objects representing the sides of the enemy cube
Quad* top_side;
Quad* right_side;
Quad* bottom_side;
Quad* left_side;
Quad* front_side;
Quad* back_side;

// A boolean integer used to determine if the explosion animation 
// currently being played.
int is_exploding;

// Floats used in calculating the rate at which the parts of the
// enemy ship explode away when the ship is hit.
float side_explosion_dist;
float side_explosion_time;
float side_explosion_move_step;

float side_explosion_total_rotation;
float side_explosion_rotate_step;
float rotation_angle;

// Cube objects used to represent the corners of the box 
// during the explosion animation
Cube* left_top_f_corner; 
Cube* right_top_f_corner; 
Cube* right_bot_f_corner; 
Cube* left_bot_f_corner; 
Cube* right_top_b_corner; 
Cube* left_top_b_corner; 
Cube* left_bot_b_corner; 
Cube* right_bot_b_corner;

// A boolean integer used to determine whether the corners should be
// drawn as cubes.
int are_corners_visible;

// Float values used to calculate movement rate for corners
float corner_dist;



// ------------------------------------
// -------> Utility Functions <--------
// ------------------------------------

// Used as a constructor to initialize a new Point object.
Point* make_point(float x, float y, float z) {
    Point* point = (Point*)malloc(sizeof(Point));

    point->x = x;
    point->y = y;
    point->z = z;

    return point;
}

// Used as a constructor to initialize a new Color object.
Color* make_color(float red, float green, float blue) {
    Color* color = (Color*)malloc(sizeof(Color));

    color->red   = red;
    color->green = green;
    color->blue  = blue;

    return color;
}

// Used as a constructor to initialize a new Color object.
Cube* make_cube(Point* center, float size, Color* color) {
    Cube* cube   = malloc(sizeof(Cube));
    cube->center = *center;
    cube->size   = size;
    cube->color  = *color;
    cube->is_alive = 0;
    cube->movement = 0;
    return cube;
}

// Used as a constructor to initialize a new Quad object.
Quad* make_quad(Point* v1, Point* v2, Point* v3, Point* v4, char axis) {
    Quad* quad = malloc(sizeof(Quad));
    quad->vertices[0] = *v1;
    quad->vertices[1] = *v2;
    quad->vertices[2] = *v3;
    quad->vertices[3] = *v4;
    quad->translation = *make_point(0.0, 0.0, 0.0);
    quad->rotation_angle = 0;
    return quad;
}

// Sets the enemy's center point to a random point along the top of 
// the canvas, and then calls itself again after a time interval 
// between 2.75 and 3.50 minutes. 
void spawn_enemy() {
    srand(time(NULL) * -time(NULL));
    enemy_spawn_x = (rand() % ((enemy_max_x+1) - enemy_min_x)) + enemy_min_x;
    enemy->center.y = enemy_start->y;
    enemy->center.x = enemy_spawn_x;

    srand(time(NULL));
    enemy_spawn_time = (rand() % (enemy_max_time+1 - enemy_min_time)) + enemy_min_time;
    enemy->is_alive = 1;
    glutTimerFunc(enemy_spawn_time, spawn_enemy, 1);
}

// Updates the enemy's center point, allowing it to move down the 
// canvas. Also checks if the enemy has reached the bottom of the
// canvas and triggers a game over if so. The method also updates
// the vertices of the side panel
void update_enemy() {
    if(enemy->is_alive) {
        enemy->center.y -= enemy_step_dist;
        if((enemy->center.y - (enemy->size / 2)) < (origin->y - (canvas_height / 2))) {
            is_game_over = 1;
        }
    }
}

// Updates the player's center point, allowing it to move along 
// the bottom of the canvas.
void update_player() {
    if (player->movement == 1 && (player->center.x >= (origin->x - (canvas_width / 2) + player->size))) {
        player->center.x -= player_step_dist;
    }
    else if (player->movement == 2 && (player->center.x <= (origin->x + (canvas_width / 2) - player->size))) {
        player->center.x += player_step_dist;
    }
}

// Adds a point to the player's score.
void add_point() {
    player_score++;
}

// Ends the explosion animation
void disable_explosion() {
    is_exploding = 0;
}

// Initiates the explosion animation 
void activate_explosion() {
    is_exploding = 1;
    glutTimerFunc(1000 * side_explosion_time, disable_explosion, 1);
}

// Kills the enemy ship, keeping it from being drawn until
// another is drawn. The sides of the cube are then initialized,
// and the explosion animation flag is triggered.
void kill_enemy() {
    Point* left_top_f = make_point(enemy->center.x - (enemy->size / 2), 
                                   enemy->center.y + (enemy->size / 2), 
                                   enemy->center.z - (enemy->size / 2));

    Point* right_top_f = make_point(enemy->center.x + (enemy->size / 2), 
                                   enemy->center.y + (enemy->size / 2), 
                                   enemy->center.z - (enemy->size / 2));

    Point* right_bot_f = make_point(enemy->center.x + (enemy->size / 2), 
                                   enemy->center.y - (enemy->size / 2), 
                                   enemy->center.z - (enemy->size / 2));

    Point* left_bot_f = make_point(enemy->center.x - (enemy->size / 2), 
                                   enemy->center.y - (enemy->size / 2), 
                                   enemy->center.z - (enemy->size / 2));

    Point* right_top_b = make_point(enemy->center.x + (enemy->size / 2), 
                                   enemy->center.y + (enemy->size / 2), 
                                   enemy->center.z + (enemy->size / 2));

    Point* left_top_b = make_point(enemy->center.x - (enemy->size / 2), 
                                   enemy->center.y + (enemy->size / 2), 
                                   enemy->center.z + (enemy->size / 2));

    Point* left_bot_b = make_point(enemy->center.x - (enemy->size / 2), 
                                   enemy->center.y - (enemy->size / 2), 
                                   enemy->center.z + (enemy->size / 2));

    Point* right_bot_b = make_point(enemy->center.x + (enemy->size / 2), 
                                   enemy->center.y - (enemy->size / 2), 
                                   enemy->center.z + (enemy->size / 2));

    top_side    = make_quad(right_top_b, left_top_b, left_top_f, right_top_f, 'x');
    top_side->center = *make_point(enemy->center.x, 
                                  enemy->center.y + (enemy->size / 2), 
                                  enemy->center.z);

    right_side  = make_quad(right_top_b, right_top_f, right_bot_f, right_bot_b, 'y');
    right_side->center = *make_point(enemy->center.x + (enemy->size / 2), 
                                    enemy->center.y, 
                                    enemy->center.z);

    bottom_side = make_quad(right_bot_f, left_bot_f, left_bot_b, right_bot_b, 'x');
    bottom_side->center = *make_point(enemy->center.x, 
                                     enemy->center.y - (enemy->size / 2), 
                                     enemy->center.z);

    left_side   = make_quad(left_top_f, left_top_b, left_bot_b, left_bot_f, 'y');
    left_side->center = *make_point(enemy->center.x - (enemy->size / 2),
                                   enemy->center.y, 
                                   enemy->center.z);

    front_side  = make_quad(right_top_f, left_top_f, left_bot_f, right_bot_f, 'y');
    front_side->center = *make_point(enemy->center.x, 
                                    enemy->center.y, 
                                    enemy->center.z - (enemy->size / 2));

    back_side   = make_quad(left_top_b, right_top_b, right_bot_b, left_bot_b, 'y');
    back_side->center = *make_point(enemy->center.x, 
                                   enemy->center.y, 
                                   enemy->center.z + (enemy->size / 2));

    left_top_f_corner = make_cube(left_top_f, 3, corner_color);
    right_top_f_corner = make_cube(right_top_f, 3, corner_color);
    right_bot_f_corner = make_cube(right_bot_f, 3, corner_color);
    left_bot_f_corner = make_cube(left_bot_f, 3, corner_color);
    right_top_b_corner = make_cube(right_top_b, 3, corner_color);
    left_top_b_corner = make_cube(left_top_b, 3, corner_color);
    left_bot_b_corner = make_cube(left_bot_b, 3, corner_color);
    right_bot_b_corner = make_cube(right_bot_b, 3, corner_color);


    activate_explosion();
    are_corners_visible = 1;
    enemy->is_alive = 0;
}

// Checks to see if the laser has been fired within the hitbox of the
// enemy ship. Kills the enemy and adds a point to the player's score
// if the hit is successful.
void test_hit() {
    if(player->center.x < enemy->center.x + (enemy->size / 2) &&
       player->center.x > enemy->center.x - (enemy->size / 2)) {
        kill_enemy();
        add_point();
    }
}

// Disables the laser from being drawn.
void disable_laser() {
    is_laser_firing = 0;
}

// Activates the drawing of the laser, initiates a check to see if the
// enemy ship has been hit, and disables drawing of the laser after
// 0.15 seconds.
void activate_laser() {
    if(is_laser_firing == 0) {
        is_laser_firing = 1;
        test_hit();
        glutTimerFunc(1000 * (0.15), disable_laser, 1);
    }
}

// 
void update_sides() {
    if(is_exploding) {
        top_side->translation.y    += side_explosion_move_step;
        right_side->translation.x  += side_explosion_move_step;
        bottom_side->translation.y -= side_explosion_move_step;
        left_side->translation.x   -= side_explosion_move_step;
        front_side->translation.z  += side_explosion_move_step;
        back_side->translation.z   -= side_explosion_move_step;

        rotation_angle += side_explosion_rotate_step;
    }
}

// Checks to see if the corners are 20 units away from their
// original positions
void check_distance() {
    if(sqrt((corner_dist * corner_dist) + (corner_dist + corner_dist)) >= 20) {
        are_corners_visible = 0;
        corner_dist = 0;
    }
}

// 
void update_corners() {
    if(are_corners_visible) {
        corner_dist += 2;
        check_distance();
    }
}


// ------------------------------------
// -------> Drawing Functions <--------
// ------------------------------------

// Draws a Cube object at the translated point stored in its 
// center field.
void draw_cube(Cube* cube) {
    glPushMatrix();
    glTranslatef(cube->center.x,
                 cube->center.y,
                 cube->center.z);
    glutSolidCube(cube->size);
    glTranslatef((cube->center.x * -1),
                 (cube->center.y * -1),
                 (cube->center.z * -1));
    glPopMatrix();
}

// Draws a laser line at the translated point stored in the Cube 
// parameter's center field.
void draw_laser(Cube* cube) {
    glBegin(GL_LINES);
        glVertex3f(cube->center.x,
                   cube->center.y + (cube->size),
                   z_plane + 15);
        glVertex3f(cube->center.x, 
                   (float)canvas_height,
                   z_plane + 15);
    glEnd();
}

// Draws a quad onto the screen
void draw_quad(Quad* quad, char axis) {
    glPushMatrix();
    // Translate during explosion
    glTranslatef(quad->translation.x,
                 quad->translation.y,
                 quad->translation.z);

    // Rotate about correct axis
     glTranslatef(quad->center.x,
                  quad->center.y,
                  quad->center.z);
     if(axis == 'x') {
         glRotatef(rotation_angle, 1.0, 0.0, 0.0);
     }
     else if(axis == 'y') {
         glRotatef(rotation_angle, 0.0, 1.0, 0.0);
     }
     else if(axis == 'z') {
         glRotatef(rotation_angle, 0.0, 0.0, 1.0);
     }
     glTranslatef(-quad->center.x,
                  -quad->center.y,
                  -quad->center.z);

    
    glBegin(GL_QUADS);
        glVertex3fv(&quad->vertices[0]);
        glVertex3fv(&quad->vertices[1]);
        glVertex3fv(&quad->vertices[2]);
        glVertex3fv(&quad->vertices[3]);
    glEnd();

    glPopMatrix();
}

// Draws the corners during the explosion animation
void draw_corners() {
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(left_top_f_corner->center.x - corner_dist,
                 left_top_f_corner->center.y + corner_dist,
                 left_top_f_corner->center.z - corner_dist);
    glutSolidCube(left_top_f_corner->size);
    glPopMatrix();

    glPushMatrix();
    glLoadIdentity();
    glTranslatef(right_top_f_corner->center.x + corner_dist,
                 right_top_f_corner->center.y + corner_dist,
                 right_top_f_corner->center.z - corner_dist);
    glutSolidCube(right_top_f_corner->size);
    glPopMatrix();

    glPushMatrix();
    glLoadIdentity();
    glTranslatef(right_bot_f_corner->center.x + corner_dist,
                 right_bot_f_corner->center.y - corner_dist,
                 right_bot_f_corner->center.z - corner_dist);
    glutSolidCube(right_bot_f_corner->size);
    glPopMatrix();

    glPushMatrix();
    glLoadIdentity();
    glTranslatef(left_bot_f_corner->center.x - corner_dist,
                 left_bot_f_corner->center.y - corner_dist,
                 left_bot_f_corner->center.z - corner_dist);
    glutSolidCube(left_bot_f_corner->size);
    glPopMatrix();

    glPushMatrix();
    glLoadIdentity();
    glTranslatef(left_top_b_corner->center.x - corner_dist,
                 left_top_b_corner->center.y + corner_dist,
                 left_top_b_corner->center.z + corner_dist);
    glutSolidCube(left_top_b_corner->size);
    glPopMatrix();

    glPushMatrix();
    glLoadIdentity();
    glTranslatef(right_top_b_corner->center.x + corner_dist,
                 right_top_b_corner->center.y + corner_dist,
                 right_top_b_corner->center.z + corner_dist);
    glutSolidCube(right_top_b_corner->size);
    glPopMatrix();

    glPushMatrix();
    glLoadIdentity();
    glTranslatef(right_bot_b_corner->center.x + corner_dist,
                 right_bot_b_corner->center.y - corner_dist,
                 right_bot_b_corner->center.z + corner_dist);
    glutSolidCube(right_bot_b_corner->size);
    glPopMatrix();

    glPushMatrix();
    glLoadIdentity();
    glTranslatef(left_bot_b_corner->center.x - corner_dist,
                 left_bot_b_corner->center.y - corner_dist,
                 left_bot_b_corner->center.z + corner_dist);
    glutSolidCube(left_bot_b_corner->size);
    glPopMatrix();
}

// Draws the scoreboard onto the top right of the canvas.
void draw_scoreboard() {
    glRasterPos3f(125.0, 280.0, z_plane + 15);
    char *string = "Score: ";
    char *c;
    for (c = string; *c != '\0'; c++)
    {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);
    }

    char* score[20];
    sprintf(score, "%d", player_score);
    for (c = score; *c != '\0'; c++)
    {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);
    }
}

// Lighting is enabled, ambient diffuse, specular, and light position are set
// up, light0 is activated, and light model local viewer is turned on.
// CITATION:
// This method of activating light comes from the textbook on pages 
// 426-427.
void light_init() {
    glEnable(GL_LIGHTING);

    float diff_light_value[] = {1.0, 1.0, 1.0, 1.0};
    float ambi_light_value[] = {0.5, 0.5, 0.5, 1.0};
    float light_position[]   = {origin->x, origin->y, origin->z, 1.0};

    glLightfv(GL_LIGHT0, GL_DIFFUSE, diff_light_value);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambi_light_value);
    glLightfv(GL_LIGHT0, GL_SPECULAR, diff_light_value);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glEnable(GL_LIGHT0);

    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
}

// Draws all of the objects onto the canvas. Materials are also set here.
// CITATION:
// This method of setting materials comes from the textbook on pages 
// 426-427.
void draw_all_objects() {
    float player_diffuse[] = { 0.0, 0.0, 0.0, 1.0 };
    float enemy_ambient[] = { 1.0, 0.0, 0.0, 1.0 };
    float enemy_diffuse[] = { 0.8, 0.0, 0.0, 1.0 };
    float corners_diffuse[] = { 0.0, 0.9, 0.0, 1.0 };
    float specular[] = { 1.0, 1.0, 1.0, 1.0 };
    float shine[] = { 50.0 };
    glClearColor(bg_color->red, bg_color->green, bg_color->blue, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    draw_scoreboard();
    light_init();
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, player_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shine);
    draw_cube(player);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, enemy_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, enemy_diffuse);
    if(enemy->is_alive) {
        draw_cube(enemy);
    }
    if(is_exploding) {
        draw_quad(front_side, 'y');
        draw_quad(right_side, 'y');
        draw_quad(back_side, 'y');
        draw_quad(left_side, 'y');
        draw_quad(top_side, 'x');
        draw_quad(bottom_side, 'x');
    }
    if(are_corners_visible) {
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, corners_diffuse);
        draw_corners();
    }
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, player_diffuse);
    if(is_laser_firing) {
        
        draw_laser(player);
    }
    glutSwapBuffers();
}

// Draws a game over message when the player has failed to kill the 
// enemy before it reached the bottom of the canvas.
void draw_game_over() {
    glClearColor(1.0, 1.0, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glRasterPos3f(-80.0, 0.0, z_plane + 15);
    char *string = "Too Bad! You Lost...";
    char *c;
    for (c = string; *c != '\0'; c++)
    {
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);
    }
    glutSwapBuffers();
}


// If the game is not currently in a game over state, then all the 
// objects will be drawn, centers of both the enemy and player will
// be updated, and then the function will be called again after a
// time interval equal to the current frame rate has passed.
void animate() {
    if(!is_game_over) {
        draw_all_objects();
        update_enemy();
        update_player();
        update_sides();
        update_corners();
        glutTimerFunc(1000 * frame_rate, animate, 1);
    }
    else {
        draw_game_over();
    }
}

// 
void initial_draw() {
    spawn_enemy();
    glutTimerFunc(1000 * frame_rate, animate, 1);
}



// ------------------------------------
// -----> User Input Functions <-------
// ------------------------------------

// Allows the game to be controlled using the keyboard. 
//      - The 'H' key moves the player left.
//      - The 'L' key moves the player right.
//      - The spacebar fires the laser.
//      - The 'Q' key quits the game.
void handle_keys(unsigned char c, GLint x, GLint y) {
    if(c == 'h' || c == 'h') {
        player->movement = 1;
    }
    else if(c == 'l' || c == 'L') {
        player->movement = 2;
    }
    else if(c == ' ') {
        activate_laser();
    }
    else if ((c == 'q') || (c == 'Q'))
    {
        exit(0);
    }
}

// Stops the player's movement when the 'H' or 'L' keys are no longer
// being pressed.
void handle_keys_up(unsigned char c, GLint x, GLint y) {
    if(c == 'h' || c == 'h') {
        player->movement = 0;
    }
    else if(c == 'l' || c == 'L') {
        player->movement = 0;
    }
}



// ------------------------------------
// --------> Main Functions <----------
// ------------------------------------

// Initializes the objects and variables that will be used.
void init() {
    origin = make_point(0.0, 0.0, 0.0);
    z_plane = -25.0;
    frame_rate = 1.0 / 30.0;

    player_size = 25.0;
    enemy_size  = 25.0;

    bg_color     = make_color(1.0, 1.0, 1.0);
    player_color = make_color(0.0, 0.0, 0.0);
    enemy_color  = make_color(0.9, 0.1, 0.1);
    corner_color = make_color(0.0, 0.9, 0.0);

    player_start = make_point(origin->x,
                              origin->y - (canvas_height / 2) + (player_size / 2),
                              z_plane);
    enemy_start  = make_point(origin->x,
                              origin->y + (canvas_height / 2) + (enemy_size / 2),
                              z_plane);

    player = make_cube(player_start, 25.0, player_color);
    enemy  = make_cube(enemy_start, 25.0, enemy_color);

    enemy_min_x = origin->x - (canvas_width / 2.0) + enemy->size / 2.0;
    enemy_max_x = origin->x + (canvas_width / 2.0) - enemy->size / 2.0;

    enemy_spawn_x = 0.0;
    enemy_spawn_time = 0.0;

    // Minimum and Maximum times in Milliseconds.
    enemy_min_time = 3000.0;
    enemy_max_time = 3500.0;

    // enemy animation rate calculation
    enemy_total_dist = (canvas_height - enemy->size);
    enemy_total_time = 2.75;
    enemy_step_dist  = (enemy_total_dist / enemy_total_time) * frame_rate;

    // player animation rate calculation
    player_total_dist = (canvas_width - player->size);
    player_total_time = 1.25;
    player_step_dist  = (player_total_dist / player_total_time) * frame_rate;

    player_score = 0;

    is_laser_firing = 0;

    is_game_over = 0;

    side_explosion_dist = 30.0;
    side_explosion_time = 0.25;
    side_explosion_move_step = (side_explosion_dist / side_explosion_time) * frame_rate;

    side_explosion_total_rotation = 360.0;
    side_explosion_rotate_step = (side_explosion_total_rotation / side_explosion_time) * frame_rate;

    are_corners_visible = 0;
    corner_dist = 0;
}



int main(int argc, char** argv) {
    init();
    glutInit(&argc, argv);
    my_setup(canvas_width, canvas_height, canvas_name);
    glutDisplayFunc(initial_draw);
    glutKeyboardFunc(handle_keys);
    glutKeyboardUpFunc(handle_keys_up);
    glutMainLoop();
    return 0;
}