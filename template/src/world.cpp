// Header
#include "world.hpp"

// stlib
#include <string.h>
#include <cassert>
#include <sstream>
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>

// Same as static in c, local to compilation unit
namespace
{
const size_t prince_acceleration_const = 400;
namespace
{
void glfw_err_cb(int error, const char *desc)
{
    fprintf(stderr, "%d: %s", error, desc);
}
} // namespace
} // namespace

World::World() : m_next_portal_spawn(0.f),
                 m_next_star_spawn(0.f),
                 m_next_wall_spawn(0.f),
                 m_next_heart_spawn(0.f)
{
    // Seeding rng with random device
    m_rng = std::default_random_engine(std::random_device()());
}

World::~World()
{
}

// World initialization
bool World::init(vec2 screen)
{
    //-------------------------------------------------------------------------
    // GLFW / OGL Initialization
    // Core Opengl 3.
    glfwSetErrorCallback(glfw_err_cb);
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
#if __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    glfwWindowHint(GLFW_RESIZABLE, 0);
    m_window = glfwCreateWindow((int)screen.x, (int)screen.y, "The Little Prince: The Lost Rose", nullptr, nullptr);
    if (m_window == nullptr)
        return false;

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // vsync

    // Load OpenGL function pointers
    gl3w_init();

    // Setting callbacks to member functions (that's why the redirect is needed)
    // Input is handled using GLFW, for more info see
    // http://www.glfw.org/docs/latest/input_guide.html
    glfwSetWindowUserPointer(m_window, this);
    auto key_redirect = [](GLFWwindow *wnd, int _0, int _1, int _2, int _3) {
        ((World *)glfwGetWindowUserPointer(wnd))->on_key(wnd, _0, _1, _2, _3);
    };
    glfwSetKeyCallback(m_window, key_redirect);

    // Create a frame buffer
    m_frame_buffer = 0;
    glGenFramebuffers(1, &m_frame_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer);

    // For some high DPI displays (ex. Retina Display on Macbooks)
    // https://stackoverflow.com/questions/36672935/why-retina-screen-coordinate-value-is-twice-the-value-of-pixel-value
    int fb_width, fb_height;
    glfwGetFramebufferSize(m_window, &fb_width, &fb_height);
    m_screen_scale = static_cast<float>(fb_width) / screen.x;

    // Initialize the screen texture
    m_screen_tex.create_from_screen(m_window);

    // Level state machine stuff
    game_state = game_start;
    n_key_press = false;
    c_key_press = false;
    y_key_press = false;
    no_key_press = false;
    escape_key_press = false;
    // q_key_press = false;
    enter_key_press = false;

    dead_pause = 50.f;

    //Initialize the grid
    float wi = screen.x / 12;
    float hi = screen.y / 8;

    for (int i = 0; i < 12; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            grid[i][j] = {wi / 2 + i * wi, hi / 2 + j * hi};
        }
    }

    using namespace std;

    //-------------------------------------------------------------------------
    // Loading music and sounds
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        fprintf(stderr, "Failed to initialize SDL Audio");
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1)
    {
        fprintf(stderr, "Failed to open audio device");
        return false;
    }

    //GET SOUNDS FOR THIS
    m_background_music = Mix_LoadMUS(audio_path("music.wav"));
    m_prince_dead_sound = Mix_LoadWAV(audio_path("salmon_dead.wav"));
    m_portal_through_sound = Mix_LoadWAV(audio_path("salmon_eat.wav"));
    //Sound works for these
    m_star_collect_sound = Mix_LoadWAV(audio_path("get_point.wav"));
    m_heart_collect_sound = Mix_LoadWAV(audio_path("get_heart.wav"));
    m_lose_heart_sound = Mix_LoadWAV(audio_path("minus_life.wav"));

    if (m_background_music == nullptr || m_portal_through_sound == nullptr || m_prince_dead_sound == nullptr ||
        m_star_collect_sound == nullptr || m_heart_collect_sound == nullptr || m_lose_heart_sound == nullptr)
    {
        fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n %s\n make sure the data directory is present",
                audio_path("music.wav"),
                audio_path("add_life.wav"),
                audio_path("salmon_dead.wav"),
                audio_path("salmon_eat.wav"));
        return false;
    }

    // Playing background music indefinitely
    Mix_PlayMusic(m_background_music, -1);

    //fprintf(stderr, "Loaded music\n");

    m_current_speed = 1.f;
    init_level(current_level);
    parse_view_file(0, 0);
    assign_background();
    //dy playable
    point_0.init();
    point_1.init();
    point_0.set_position(vec2{120, 60});
    point_1.set_position(vec2{80, 60});
    star_bar.init();
    star_bar.change_state(3);
    star_bar.set_position(vec2{40, 80});
    return m_prince.init() && m_background.init() && m_pebbles_emitter.init() && set_bars() &&
           arrow.init(); //DY playable
}

// Releases all the associated resources
void World::destroy()
{
    glDeleteFramebuffers(1, &m_frame_buffer);

    Mix_CloseAudio();

    if (m_prince_dead_sound != nullptr)
        Mix_FreeChunk(m_prince_dead_sound);
    if (m_star_collect_sound != nullptr)
        Mix_FreeChunk(m_star_collect_sound);
    if (m_portal_through_sound != nullptr)
        Mix_FreeChunk(m_portal_through_sound);
    if (m_heart_collect_sound != nullptr)
        Mix_FreeChunk(m_heart_collect_sound);
    if (m_lose_heart_sound != nullptr)
        Mix_FreeChunk(m_lose_heart_sound);
    if (m_background_music != nullptr)
        Mix_FreeMusic(m_background_music);

    m_prince.destroy();
    m_pebbles_emitter.destroy();

    for (auto &heart : m_hearts_bar)
        heart.destroy();
    m_hearts_bar.clear();
    for (auto &heart : m_hearts_grey_bar)
        heart.destroy();
    m_hearts_grey_bar.clear();

    glfwDestroyWindow(m_window);
}

bool World::update(float elapsed_ms)
{
    int w, h;
    glfwGetFramebufferSize(m_window, &w, &h);
    vec2 screen = {(float)w / m_screen_scale, (float)h / m_screen_scale};

    // CHECK GAME STATE; 0 = start screen, 1 = game play, 2 = game load, 3 = end screen
    if (game_state == game_start)
    {
        if (c_key_press || n_key_press)
        {
            update_screen(4);
            game_state = game_play;
        }
        return true;
    }
    else if (game_state == game_play)
    {
        int curr_view = get_curr_view();
        check_collisions(elapsed_ms);

        // check if prince has collected all the stars - if yes then protal to next level appears
        if (m_prince.is_alive() && is_ready_next_level() && curr_view == m_level.portal_view &&
            CollisionManager::prince_collides_with_portal(&m_prince,
                                                          m_level.camera_views[curr_view].m_opt_portal.m_portal))
        {
            game_state = game_level_screen;
            // go_next_level();
        }

        vec2 little_prince_position = m_prince.get_position();

        if (little_prince_position.x > 1200)
        {
            m_level.current_view.col += 1;
            update_screen(3);
        }
        else if (little_prince_position.x < 0)
        {
            m_level.current_view.col -= 1;
            update_screen(2);
        }
        else if (little_prince_position.y > 800)
        {
            m_level.current_view.row += 1;
            update_screen(1);
        }
        else if (little_prince_position.y < 0)
        {
            m_level.current_view.row -= 1;
            update_screen(0);
        }

        m_prince.update(elapsed_ms);
        // create smoke when prince is running
        if (len(m_prince.m_velocity) > 0.f)
        {
            if (m_prince.m_velocity.x < 0.f)                                  // left
                m_pebbles_emitter.spawn_pebble(m_prince.get_position(), 0);   // smoke on right
            else if (m_prince.m_velocity.x > 0.f)                             // right
                m_pebbles_emitter.spawn_pebble(m_prince.get_position(), 180); // smoke on left
            else if (m_prince.m_velocity.y < 0.f)                             // up
                m_pebbles_emitter.spawn_pebble(m_prince.get_position(), 270); // smoke on down
            else if (m_prince.m_velocity.y > 0.f)                             // down
                m_pebbles_emitter.spawn_pebble(m_prince.get_position(), 90);  // smoke on up
        }
        m_pebbles_emitter.update(elapsed_ms);

        for (auto &star : m_level.camera_views[curr_view].m_stars)
        {
            star.update(0); //DY
        }
        //dy playable
        float point0 = (int)m_prince.m_points % 10;
        float point1 = (int)m_prince.m_points / 10;
        point_0.update(point0);
        point_1.update(point1);
        star_bar.update(0);
        arrow.update(elapsed_ms * m_current_speed);
        for (auto &heart : m_hearts_bar)
            heart.update(elapsed_ms * m_current_speed);
        for (auto &heart : m_level.camera_views[curr_view].m_hearts)
            heart.update(elapsed_ms * m_current_speed);
        for (auto &wall : m_walls)
            wall.update(elapsed_ms * m_current_speed);
        if (m_level.camera_views[curr_view].m_opt_key.is_this_view)
            m_level.camera_views[curr_view].m_opt_key.m_key.update();
        if (m_level.camera_views[curr_view].m_opt_portal.is_this_view)
            m_level.camera_views[curr_view].m_opt_portal.m_portal.update();
        for (auto &enemy : m_level.camera_views[curr_view].m_enemies)
        {
            enemy.check_distance(m_prince.get_position());
            enemy.update(elapsed_ms * m_current_speed);
            Bullet *bullet = &m_bullets_enemy[enemy.get_key()];
            if (enemy.get_stop_and_attack())
            {
                bullet->set_shooting(true);
                if (!bullet->get_runnning())
                {
                    bullet->set_position(enemy.get_position());
                    bullet->set_rotation(enemy.get_angle());
                }
            }
            bullet->update(elapsed_ms);
        }

        for (auto &prince_bullet : m_bullets_prince_)
            prince_bullet.update(elapsed_ms);

        return true;
    }

    else if (game_state == game_level_screen)
    {

        // set change background to true
        m_background.change_background();
        if (enter_key_press)
        {
            go_next_level();
        }
    }
    else if (game_state == game_quit_screen)
    {
        m_background.change_background();
        if (y_key_press)
        {
            // clear camera view, then destroy game
            clear_camera_view();
            game_state = game_start;
            y_key_press = false;
        }
        else if (no_key_press)
        {
            game_state = game_play;
        }
    }
    else if (game_state == game_help_screen)
    {
        m_background.change_background();
        if (escape_key_press)
        {
            switch (last_state)
            {
            case 0:
                game_state = game_start;
                break;
            case 1:
                game_state = game_quit_screen;
                break;
            case 2:
                game_state = game_level_screen;
                break;
            case 3:
                game_state = game_end;
                break;
            default:
                game_state = game_play;
            }
            escape_key_press = false;
        }
    }
    else if (game_state == game_dead)
    {
        m_background.change_background();
        load_save_file();
        if (dead_pause == 0.f)
        {

            dead_pause = 50.f;
            game_state = game_play;
            update_screen(4);
        }
        else
        {
            dead_pause -= 1.0f;
        }
    }
}

void World::check_collisions(float ms)
{
    int curr_view = get_curr_view();
    // Check Enemy and Prince collision
    for (auto &enemy : m_level.camera_views[curr_view].m_enemies)
    {
        if (CollisionManager::prince_collides_with_enemy(&m_prince, &enemy))
        {
            if (!m_prince.is_extra_lives() && m_prince.is_alive() && current_level != 0)
            // prince is dead now; play dead sound
            {
                game_state = game_dead;
                break;
            }
            else
            {
                if (m_prince.is_extra_lives())
                {
                    if (current_level == 0)
                    {
                        m_prince.reset_position(3);
                        vec2 pos = {600.f, 400.f};
                        enemy.set_position(pos);
                        m_pebbles_emitter.destroy();
                        m_pebbles_emitter.init();
                    }
                    else
                    {
                        m_prince.minus_live();
                        enemy.set_position(enemy.get_guard_position());
                        //Dy: pop the last heart bar if dead once
                    }
                    // prince has extra lives, so minus one life; play life down sound
                    Mix_PlayChannel(-1, m_lose_heart_sound, 0);
                }
            }
        }
        Bullet *bullet = &m_bullets_enemy[enemy.get_key()];
        if (CollisionManager::prince_collides_with_bullet(&m_prince, bullet))
        {
            if (!m_prince.is_extra_lives() && m_prince.is_alive() && current_level != 0)
            // prince is dead now; play dead sound
            {
                //m_prince.kill();
                game_state = game_dead;
                //game_state = game_try_again;
                break;
                // Mix_PlayChannel(-1, m_salmon_dead_sound, 0); //play prince dead sound
                // m_water.set_salmon_dead();  // change background color when prince is dead
            }
            else
            {
                if (m_prince.is_extra_lives())
                {
                    float radians = enemy.get_angle();
                    float xp = 5.0f * sin(radians);
                    float yp = -5.0f * cos(radians);
                    m_prince.move(vec2{xp, yp});
                    if (current_level == 0)
                    {
                        vec2 pos = {600.f, 400.f};
                        enemy.set_position(pos);
                        bullet->set_position(enemy.get_position());
                        // bullet->set_running(false);
                        // bullet->set_shooting(false);
                        bullet->set_finished_shotting(true);
                    }
                    else
                    {
                        m_prince.minus_live();
                        //Dy: pop the last heart bar if dead once
                        m_hearts_bar.pop_back();
                        bullet->set_position(enemy.get_position());
                        // bullet->set_running(false);
                        // bullet->set_shooting(false);
                        bullet->set_finished_shotting(true);
                    }
                    // prince has extra lives, so minus one life; play life down sound
                    Mix_PlayChannel(-1, m_lose_heart_sound, 0);
                }
            }
            // advanced = false;
            // m_prince.m_advanced = false;
            // m_prince.kill();
        }
        if (CollisionManager::bullet_collides_with_boundry(bullet))
        {
            bullet->set_position(enemy.get_position());
            // bullet->set_running(false);
            // bullet->set_shooting(false);
            bullet->set_finished_shotting(true);
        }
    }
    //DY PLAYABLE
    // Checking Prince - Star collisions
    auto star_it = m_level.camera_views[curr_view].m_stars.begin();
    while (star_it != m_level.camera_views[curr_view].m_stars.end())
    {
        if (m_prince.is_alive() && CollisionManager::prince_collides_with_star(&m_prince, *star_it) &&
            star_it->get_star_state() == 0)
        {
            Mix_PlayChannel(-1, m_star_collect_sound, 0);
            star_it->move(vec2{(star_it->get_position().x - m_prince.get_position().x),
                               (star_it->get_position().y - m_prince.get_position().y)});
            ++m_prince.m_points;
            star_it->fly_on = m_prince.m_points + 1; //DY
        }
        else
            ++star_it;
    }
    // Checking should delete the star from bar DY
    auto star_it_bar = m_level.camera_views[curr_view].m_stars.begin();
    while (star_it_bar != m_level.camera_views[curr_view].m_stars.end())
    {
        if (m_prince.is_alive() && star_it_bar->get_star_state() == 2)
        {
            star_it_bar = m_level.camera_views[curr_view].m_stars.erase(star_it_bar);
        }
        else
            ++star_it_bar;
    }

    // check prince - key collision
    if (m_prince.is_alive() && curr_view == m_level.key_view &&
        m_level.camera_views[curr_view].m_opt_key.is_this_view &&
        CollisionManager::prince_collides_with_key(&m_prince, m_level.camera_views[curr_view].m_opt_key.m_key))
    {
        m_level.camera_views[curr_view].m_opt_key.m_key.set_is_collected(true);
        m_level.camera_views[curr_view].m_opt_key.is_this_view = false;
        if (current_level == 0)
        {
            m_level.camera_views[curr_view].m_opt_portal.m_portal.init();
            m_level.camera_views[curr_view].m_opt_portal.m_portal.set_position(
                add(m_level.camera_views[curr_view].m_opt_key.m_key.get_position(), {200.f, 0.f}));
        }
        else
        {
            spawn_portal();
        }

        Mix_PlayChannel(-1, m_star_collect_sound, 0);
    }

    // Checking Prince - Heart collisions
    auto heart_it = m_level.camera_views[curr_view].m_hearts.begin();
    while (heart_it != m_level.camera_views[curr_view].m_hearts.end())
    {
        if (m_prince.is_alive() && CollisionManager::prince_collides_with_heart(&m_prince, *heart_it))
        {
            heart_it = m_level.camera_views[curr_view].m_hearts.erase(heart_it);
            Mix_PlayChannel(-1, m_heart_collect_sound, 0);

            // keeps track of prince lives, if max 5 then don't add any more lives
            // m_prince.life_count += 1;
            if (m_prince.get_lives() < 5)
            {
                m_prince.add_life();
                // add_heart_bar();
            }
        }
        else
        {
            ++heart_it;
        }
    }

    // Checking Prince - wall collisions
    auto wall_it = m_walls.begin();
    while (wall_it != m_walls.end())
    {
        if (m_prince.is_alive() && CollisionManager::prince_collides_with_wall(&m_prince, *wall_it, ms))
        {
            float vector_x = m_prince.get_position().x - wall_it->get_position().x;
            float vector_y = m_prince.get_position().y - wall_it->get_position().y;
            vec2 normalize_vector = normalize({vector_x, vector_y});
            m_prince.move(mul(normalize_vector, 6));
            m_prince.m_velocity = {0.f, 0.f};
            break;
        }
        else
        {
            for (auto &enemy : m_level.camera_views[curr_view].m_enemies)
            {
                if (enemy.collides_with(*wall_it, ms))
                {
                    vec2 vector = sub(enemy.get_guard_position(), enemy.get_position());
                    if (len(vector) < 0.001f)
                        break;
                    vec2 normalize_vector = normalize(vector);
                    enemy.move(mul(normalize_vector, 2));
                    break;
                }
            }
            ++wall_it;
        }
    }
    auto bullet_it = m_bullets_prince_.begin();
    while (bullet_it != m_bullets_prince_.end())
    {
        bool continue_running = false;
        auto enemy = m_level.camera_views[curr_view].m_enemies.begin();
        while (enemy != m_level.camera_views[curr_view].m_enemies.end())
        {
            if (CollisionManager::bullet_collides_with_enemy(*bullet_it, *enemy))
            {
                float bullet_radians = bullet_it->get_radians();
                bullet_it = m_bullets_prince_.erase(bullet_it);
                enemy->minus_life();
                continue_running = true;
                if (!enemy->enemy_kill())
                {
                    float x = 5.0f * sin(bullet_radians);
                    float y = -5.0f * cos(bullet_radians);
                    enemy->move(vec2{x, y});
                }
                else
                {
                    m_level.camera_views[curr_view].m_enemies.erase(enemy);
                    break;
                }
            }
            ++enemy;
        }
        if (continue_running)
            continue;
        for (auto &walls : m_walls)
        {
            if (CollisionManager::bullet_collides_with_wall(&*bullet_it, walls))
            {
                bullet_it = m_bullets_prince_.erase(bullet_it);
                continue_running = true;
                m_prince.m_points += walls.get_points(); //final dy
                break;
            }
        }
        if (continue_running)
            continue;
        if (CollisionManager::bullet_collides_with_boundry(&*bullet_it))
        {
            bullet_it = m_bullets_prince_.erase(bullet_it);
            continue_running = true;
        }
        if (continue_running)
            continue;
        else
            ++bullet_it;
    }
    // >>>>>>> playable-steph
}

void World::update_screen(int pos)
{
    m_background.change_background();
    m_prince.reset_position(pos);
    parse_view_file(m_level.current_view.row, m_level.current_view.col);
    for (auto &wall : m_walls)
        wall.destroy();
    m_walls.clear();
    Walls::set_texture_change();
    assign_background();
    m_pebbles_emitter.destroy();
    m_pebbles_emitter.init();
    for (auto &bullet : m_bullets_prince_)
        bullet.destroy();
    m_bullets_prince_.clear();
    Enemy::set_texture_change();
}

void World::draw()
{
    // Clearing error  buffer
    gl_flush_errors();

    // Getting size of window
    int w, h;
    glfwGetFramebufferSize(m_window, &w, &h);

    // Updating window title with points
    std::stringstream title_ss;
    title_ss << "The Little Prince and the Lost Rose" << m_prince.m_points;
    glfwSetWindowTitle(m_window, title_ss.str().c_str());

    /////////////////////////////////////
    // First render to the custom framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer);

    // Clearing backbuffer
    glViewport(0, 0, w, h);
    glDepthRange(0.00001, 10);
    const float clear_color[3] = {0.3f, 0.3f, 0.8f};
    glClearColor(clear_color[0], clear_color[1], clear_color[2], 1.0);
    glClearDepth(1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Fake projection matrix, scales with respect to window coordinates
    // PS: 1.f / w in [1][1] is correct.. do you know why ? (:
    float left = 0.f;                         // *-0.5;
    float top = 0.f;                          // (float)h * -0.5;
    float right = (float)w / m_screen_scale;  // *0.5;
    float bottom = (float)h / m_screen_scale; // *0.5;

    float sx = 2.f / (right - left);
    float sy = 2.f / (top - bottom);
    float tx = -(right + left) / (right - left);
    float ty = -(top + bottom) / (top - bottom);
    mat3 projection_2D{{sx, 0.f, 0.f},
                       {0.f, sy, 0.f},
                       {tx, ty, 1.f}};

    /////////////////////
    // Truely render to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Clearing backbuffer
    glViewport(0, 0, w, h);
    glDepthRange(0, 10);
    glClearColor(0, 0, 0, 1.0);
    glClearDepth(1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Need to create the img path before drawing the background texture

    if (game_state == game_start)
    {

        m_background.change_background();
        m_background.set_intro_img_path();
        // for some reason prince always has to be drawn otherwise the screen won't load ):
        m_prince.set_position(vec2{-200, -200});
        m_prince.draw(projection_2D);

        m_background.draw(projection_2D);
        //        for(auto& wall : m_walls)
        //            wall.destroy();
        //        m_walls.clear();
    }
    else if (game_state == game_play)
    {

        m_background.create_img_path(current_level, m_level.current_view.row, m_level.current_view.col);
        m_background.draw(projection_2D);

        for (auto &wall : m_walls)
            wall.draw(projection_2D);

        // Drawing entities
        int curr_view = get_curr_view();
        for (auto &star : m_level.camera_views[curr_view].m_stars)
            star.draw(projection_2D);
        for (auto &heart : m_level.camera_views[curr_view].m_hearts)
            heart.draw(projection_2D);
        for (auto &bullet : m_bullets_prince_)
            bullet.draw(projection_2D);
        if (current_level == 0 && get_curr_view() != 3)
        {
            arrow.draw(projection_2D);
        }
        m_pebbles_emitter.draw(projection_2D);
        m_prince.draw(projection_2D);
        if (m_level.camera_views[curr_view].m_opt_key.is_this_view)
            m_level.camera_views[curr_view].m_opt_key.m_key.draw(projection_2D);

        for (auto &enemy : m_level.camera_views[curr_view].m_enemies)
        {
            enemy.draw(projection_2D);
            Bullet *bullet = &m_bullets_enemy[enemy.get_key()];
            if (enemy.get_stop_and_attack() || bullet->get_runnning())
            {
                bullet->draw(projection_2D);
            }
        }
        point_0.draw(projection_2D);
        point_1.draw(projection_2D);
        star_bar.draw(projection_2D);

        for (int i = 0; i < m_prince.get_lives(); i++)
        {
            m_hearts_bar[i].draw(projection_2D);
        }
        for (int i = m_prince.get_lives(); i < 5; i++)
        {
            m_hearts_grey_bar[i].draw(projection_2D);
        }

        if (is_ready_next_level() && m_level.camera_views[curr_view].m_opt_portal.is_this_view)
        {
            m_level.camera_views[curr_view].m_opt_portal.m_portal.draw(projection_2D);
        }
    }
    else if (game_state == game_level_screen)
    {
        m_background.set_level_load_img_path(current_level);
        m_prince.set_position(vec2{-200, -200});
        m_prince.draw(projection_2D);
        m_background.draw(projection_2D);
    }
    else if (game_state == game_quit_screen)
    {
        m_background.set_quit_screen_img_path();
        m_prince.set_position(vec2{-200, -200});
        m_prince.draw(projection_2D);
        m_background.draw(projection_2D);
    }
    else if (game_state == game_help_screen)
    {
        m_background.set_help_screen_img_path();
        m_prince.set_position(vec2{-200, -200});
        m_prince.draw(projection_2D);
        m_background.draw(projection_2D);
    }
    else if (game_state == game_dead)
    {
        m_prince.set_position(vec2{-200, -200});
        m_prince.draw(projection_2D);
        m_background.set_lost_game_img_path((dead_pause / 10) + 1);
        m_background.change_background();
        m_background.draw(projection_2D);
    }

    // Presenting
    glfwSwapBuffers(m_window);
}

// Should the game be over ?
bool World::is_over() const
{
    return glfwWindowShouldClose(m_window);
}

// is collect all stars then set is_next_level to true
bool World::is_ready_next_level()
{

    return m_level.camera_views[m_level.key_view].m_opt_key.m_key.get_is_collected();
}

void World::go_next_level()
{
    clear_camera_view();
    current_level += 1;
    m_level.current_view.row = 0;
    m_level.current_view.col = 0;
    m_pebbles_emitter.destroy();
    m_pebbles_emitter.init();
    m_prince.m_velocity = {0.f, 0.f};
    write_to_save_file();
    init_level(current_level);

    // update screen with dafault prince position
    update_screen(5);

    game_state = game_play;
}

int World::get_curr_view()
{
    return m_level.current_view.row * (m_level.view_grid.col) + m_level.current_view.col;
}

// Creates a key and doesn't spawn it on wall
void World::spawn_key()
{
    // printf("spawn key called\n");
    int rand_num = 0;
    vec2 rand_pos = {0.f, 0.f};
    float dist_pos = 0.f;
    bool is_too_near = true;

    while (is_too_near)
    {
        rand_num = rand() % matrix_positions.size();
        rand_pos = matrix_positions[rand_num];
        dist_pos = sq_len(sub(m_prince.get_position(), rand_pos));
        is_too_near = dist_pos <= 2.f * (float)(100 * 100);
    }

    Key new_key;
    if (new_key.init())
    {
        new_key.set_position(rand_pos);
        m_level.camera_views[m_level.key_view].m_opt_key.m_key = new_key;
        m_level.camera_views[m_level.key_view].m_opt_key.is_this_view = true;
    }
    matrix_positions.erase(matrix_positions.begin() + rand_num);
}

void World::spawn_portal()
{

    if (m_level.camera_views[m_level.portal_view].m_opt_portal.m_portal.init())
    {
        // get it near position of key
        int rand_num = 0;
        vec2 rand_pos = {0.f, 0.f};
        float dist_pos = 0.f;
        bool is_too_far = true;
        bool is_too_near = true;

        while (is_too_far || is_too_near)
        {
            rand_num = rand() % matrix_positions.size();
            rand_pos = matrix_positions[rand_num];
            dist_pos = sq_len(sub(m_level.camera_views[m_level.portal_view].m_opt_key.m_key.get_position(), rand_pos));
            is_too_far = dist_pos >= 5.f * (float)(100 * 100);
            is_too_near = dist_pos <= (float)(100 * 100);
        }

        m_level.camera_views[m_level.portal_view].m_opt_portal.is_this_view = true;
        m_level.camera_views[m_level.portal_view].m_opt_portal.m_portal.set_position(rand_pos);

        // printf("\n portal generated\n");
    }
}
bool comp(int a, int b)
{
    return (a < b);
}
void World::spawn_star(int max_star, int view_id)
{
    for (int i = 0; i < max_star; i++)
    {
        int rand_num = 0;
        vec2 rand_pos = {0.f, 0.f};
        float dist_pos = 0.f;

        bool is_too_near = true, is_on_boundary = true, is_near_prince = true;

        while (is_too_near || is_on_boundary || is_near_prince)
        {
            rand_num = rand() % matrix_positions.size();
            rand_pos = matrix_positions[rand_num];
            dist_pos = sq_len(sub(m_prince.get_position(), rand_pos));
            // the enemy and star far away from the prince star location
            is_too_near = dist_pos <= 5.f * (float)(100 * 100);
            float min_arg = std::min({rand_pos.x, 1200.f - rand_pos.x, rand_pos.y, 800.f - rand_pos.y}, comp);
            is_on_boundary = min_arg < 150.f;
            vec2 prince_next_level_position = {150.f, 150.f};
            float enemy_attack_dist = 200.f;
            is_near_prince = len(sub(prince_next_level_position, rand_pos)) < enemy_attack_dist;
        }

        Star new_star;
        if (new_star.init())
        {
            new_star.set_position(rand_pos);
            m_level.camera_views[view_id].m_stars.emplace_back(new_star);
        }
        matrix_positions.erase(matrix_positions.begin() + rand_num);
    }
}
// generate new heart
void World::spawn_heart(int max_hearts, int view_id)
{
    for (int i = 0; i < max_hearts; i++)
    {
        int rand_num = rand() % matrix_positions.size();
        vec2 rand_pos = matrix_positions[rand_num];

        float dist_pos = sq_len(sub(m_prince.get_position(), rand_pos));

        while (dist_pos <= 2.f * (float)(100 * 100))
        {
            rand_num = (rand_num + 1) % matrix_positions.size();
            rand_pos = matrix_positions[rand_num];
            dist_pos = sq_len(sub(m_prince.get_position(), rand_pos));
        }
        Heart new_heart;
        if (new_heart.init())
        {
            new_heart.set_position(rand_pos);
            m_level.camera_views[view_id].m_hearts.emplace_back(new_heart);
        }
        matrix_positions.erase(matrix_positions.begin() + rand_num);
    }
}

bool World::spawn_walls(int n)
{
    Walls wall;
    if (wall.init(n, current_level))
    {
        m_walls.emplace_back(wall);
        return true;
    }
    fprintf(stderr, "Failed to spawn walls");
    return false;
}

void World::on_key(GLFWwindow *, int key, int, int action, int mod)
{
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_LEFT)
        {
            m_prince.motion.acceleration = add(m_prince.motion.acceleration, {-1.f * prince_acceleration_const, 0.f});
        }
        else if (key == GLFW_KEY_RIGHT)
        {
            m_prince.motion.acceleration = add(m_prince.motion.acceleration, {prince_acceleration_const, 0.f});
        }
        else if (key == GLFW_KEY_UP)
        {
            m_prince.motion.acceleration = add(m_prince.motion.acceleration, {0.f, -1.f * prince_acceleration_const});
        }
        else if (key == GLFW_KEY_DOWN)
        {
            m_prince.motion.acceleration = add(m_prince.motion.acceleration, {0.f, prince_acceleration_const});
        }
        else if (key == GLFW_KEY_N)
        {

            if (game_state == game_quit_screen)
            {
                no_key_press = true;
            }
            else
            {
                n_key_press = true;
                clear_camera_view();
                Walls::set_texture_change();
                Enemy::set_texture_change();
                current_level = 0;
                m_level.current_view.row = 0;
                m_level.current_view.col = 0;
                parse_view_file(0, 0);
                m_prince.m_points = 0;
                m_prince.set_lives(3);
                // called so things can be placed
                init_level(0);
                // need to have something else here that changes the textures
                // of the enemy to match the current level
                Walls::set_texture_change();
                Enemy::set_texture_change();
                update_screen(4);
                game_state = game_play;
            }
        }
        else if (key == GLFW_KEY_C)
        {
            c_key_press = true;
            load_save_file();
            update_screen(4);
            game_state = game_play;
        }

        //         game_state = game_quit_screen;
        //     }
        //     // q_key_press = true;
        // }
        else if (key == GLFW_KEY_S)
        {
            write_to_save_file();
        }
        else if (key == GLFW_KEY_Y)
        {

            if (game_state == game_quit_screen)
            {
                y_key_press = true;
            }
        }
        else if (key == GLFW_KEY_H)
        {
            switch (game_state)
            {
            case game_start:
                last_state = 0;
                break;
            case game_quit_screen:
                last_state = 1;
                break;
            case game_level_screen:
                last_state = 2;
                break;
            case game_end:
                last_state = 3;
                break;
            default:
                last_state = 4;
            }

            game_state = game_help_screen;
        }
        else if (key == GLFW_KEY_ENTER)
        {
            enter_key_press = true;
        }
        else if (key == GLFW_KEY_ESCAPE)
        {

            if (game_state == game_help_screen)
            {
                escape_key_press = true;
            }
            else
            {
                game_state = game_quit_screen;
            }
        }
        else if (key == GLFW_KEY_SPACE && m_prince.m_points != 0)
        {
            if (!spawn_prince_bullet())
                return;
            Bullet &new_bullet = m_bullets_prince_.back();
            new_bullet.set_position(m_prince.get_position());
            if (m_prince.m_velocity.y != 0 || m_prince.m_velocity.x != 0)
                prev_prince_v = atan2(m_prince.m_velocity.y, m_prince.m_velocity.x) + M_PI / 2;
            new_bullet.set_rotation(prev_prince_v);
            new_bullet.set_running(true);
            m_prince.m_points--;
        }
    }

    // on key release
    if (action == GLFW_RELEASE)
    {
        if (key == GLFW_KEY_LEFT)
        {
            m_prince.motion.acceleration = sub(m_prince.motion.acceleration, {-1.f * prince_acceleration_const, 0.f});
        }
        else if (key == GLFW_KEY_RIGHT)
        {
            m_prince.motion.acceleration = sub(m_prince.motion.acceleration, {prince_acceleration_const, 0.f});
        }
        else if (key == GLFW_KEY_UP)
        {
            m_prince.motion.acceleration = sub(m_prince.motion.acceleration, {0.f, -1.f * prince_acceleration_const});
        }
        else if (key == GLFW_KEY_DOWN)
        {
            m_prince.motion.acceleration = sub(m_prince.motion.acceleration, {0.f, prince_acceleration_const});
        }
        else if (key == GLFW_KEY_N)
        {
            no_key_press = false;
            n_key_press = false;
        }
        else if (key == GLFW_KEY_C)
        {
            c_key_press = false;
        }
        else if (key == GLFW_KEY_Y)
        {
            y_key_press = false;
        }
        //        else if (key == GLFW_KEY_Q)
        //        {
        //            q_key_press = false;
        //        }
        else if (key == GLFW_KEY_ENTER)
        {
            enter_key_press = false;
        }
        else if (key == GLFW_KEY_ESCAPE)
        {
            //            escape_key_press = false;
        }
    }

    // Resetting game

    if (action == GLFW_RELEASE && key == GLFW_KEY_R)
    {
        int w, h;
        glfwGetWindowSize(m_window, &w, &h);
        m_prince.destroy();
        m_prince.init();
        m_prince.reset_position(5);
        matrix_positions.clear();
        m_background.destroy();
        m_background.init();
        int curr_view = get_curr_view();
        m_current_speed = 1.f;
        m_pebbles_emitter.destroy();
        clear_camera_view();
        parse_view_file(0, 0);
        assign_background();
        init_level(current_level);
    }

    // Control the current speed with `<` `>`
    if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA)
        m_current_speed -= 0.1f;
    if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD)
        m_current_speed += 0.1f;

    m_current_speed = fmax(0.f, m_current_speed);
}

// parse the current view file
void World::parse_view_file(int vrow, int vcol)
{
    std::string line;
    std::ifstream myfile;
    int row = 0;
    int col = 0;

    // Get file based on current level, row and column
    std::string file_path = std::to_string(current_level) + "/" + std::to_string(vrow) + std::to_string(vcol) + ".txt";
    std::string dir_path = data_path "/maps/";
    std::string full_path = dir_path + file_path;

    // Open the file
    myfile.open(full_path);

    // If the file cannot open, print out a message to the console
    if (!myfile.is_open())
    {
        fprintf(stderr, "Cannot open view file!\n\n");
    }

    // While there is still a line, read the next line in the file
    while (std::getline(myfile, line))
    {
        std::string delimiter = " ";
        size_t pos = 0;
        std::string token;

        // Split the line into characters by using the " " delimiter
        while ((pos = line.find(delimiter)) != std::string::npos)
        {
            // Extract the substring based on the first found delimiter
            // and add to our matrix
            token = line.substr(0, pos);
            singleview[row][col] = token;

            // Erase the current character and space from the line
            line.erase(0, pos + delimiter.length());
            col++;
        }

        // Append the last character to the singleview matrix
        // NOTE: we need this step because we erase everything else from the line,
        // so the last character has no delimiter, which means the while loop ends,
        // but we haven't appened it to our matrix yet, it's still left in the line
        singleview[row][col] = line;

        // POSSIBLE BUG HERE
        // Reset j to 0 for the next row and increment row counter
        col = 0;
        row++;
    }
    myfile.close();
    init_matrix();
}

// parse the current view file
void World::parse_sum_file(int current_level)
{
    if (current_level == 0)
    {
        return;
    }
    std::string line;
    std::ifstream myfile;

    // Get file based on current level, row and column
    //std::string file_path = "INFO/" + std::to_string(1) + ".txt";
    std::string file_path = std::to_string(current_level) + "/" + std::to_string(current_level) + ".txt";
    std::string dir_path = data_path "/maps/";
    std::string full_path = dir_path + file_path;
    //printf("%s", full_path.c_str());

    // printf("%s\n", file_path.c_str());

    // Open the file
    myfile.open(full_path);
    // If the file cannot open, print out a message to the console
    if (!myfile.is_open())
    {
        fprintf(stderr, "Cannot open INFO file!\n\n");
    }
    // While there is still a line, read the next line in the file
    while (std::getline(myfile, line))
    {
        std::string delimiter = " ";
        size_t pos = 0;
        std::string token;
        int i = 0;
        // Split the line into characters by using the " " delimiter
        while ((pos = line.find(delimiter)) != std::string::npos)
        {
            // Extract the substring based on the first found delimiter
            // and add to our matrix
            token = line.substr(0, pos);

            std::stringstream geek(token);

            // Covert into int

            geek >> level_info[i];

            // printf("here %s\n", token.c_str());
            //parsed[i] = token;
            //singleview[row][col] = token;

            // Erase the current character and space from the line
            line.erase(0, pos + delimiter.length());
            i++;
        }

        // Append the last character to the singleview matrix
        // NOTE: we need this step because we erase everything else from the line,
        // so the last character has no delimiter, which means the while loop ends,
        // but we haven't appened it to our matrix yet, it's still left in the line
        //singleview[row][col] = line;
        std::stringstream geek(line);

        // Covert into int

        geek >> level_info[i];
        //parsed[i] = line;
        // printf("\n last is %d, i is %d\n", level_info[i], i);
    }
}

void World::assign_background()
{
    printf("\n new background is being assigned now \n");
    for (int i = 0; i < 12; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if ("W" == singleview[i][j])
            {
                if (!spawn_walls(1))
                    return;
                Walls &new_wall = m_walls.back();
                if (i == 14 && j == 16)
                {
                    printf("\n end\n");
                }
                new_wall.set_position(grid[i][j]);
            }
            if ("T" == singleview[i][j])
            {
                if (!spawn_walls(2))
                    return;
                //printf("is a tree");
                Walls &new_wall = m_walls.back();

                new_wall.set_position(grid[i][j]);
            }
            if ("M" == singleview[i][j])
            {
                if (!spawn_walls(3))
                    return;
                //printf("is a tree");
                Walls &new_wall = m_walls.back();

                new_wall.set_position(grid[i][j]);
            }
        }
    }

    Walls::unset_texture_change();
}

bool World::spawn_enemy(int max_enemies, int view_id)
{
    for (int i = 0; i < max_enemies; i++)
    {
        Enemy enemy;
        Bullet enemy_bullet;
        if (enemy.init(current_level) && enemy_bullet.init(0))
        {
            float x = m_level.camera_views[view_id].m_stars[i].get_position().x;
            float y = m_level.camera_views[view_id].m_stars[i].get_position().y;
            enemy.set_position(vec2{x, y});
            enemy.set_guard_position(vec2{x, y});
            enemy.set_key(enemy_count);
            m_level.camera_views[view_id].m_enemies.emplace_back(enemy);
            enemy_bullet.set_position(enemy.get_position());
            m_bullets_enemy.emplace(enemy_count, enemy_bullet);
            ++enemy_count;
        }
        else
        {
            fprintf(stderr, "Failed to spawn enemy");
            return false;
        }
    }
    return true; //dy playable
}

bool World::spawn_prince_bullet()
{
    Bullet bullet;
    if (bullet.init(0))
    {
        m_bullets_prince_.emplace_back(bullet);
        return true;
    }
    fprintf(stderr, "Failed to spawn prince bullet");
    return false;
}

//set points bar and live bar
//TODO: try to update the number of h as lives up and down
//	try a fancy way to show points collects
bool World::set_bars()
{

    for (int j = m_hearts_bar.size(); j < 5; j++)
    {
        Heart new_heart;
        if (new_heart.init())
        {
            new_heart.set_scale(vec2{0.08f, 0.08f});
            new_heart.set_position(vec2{20 + 45.0f * j, 20});
            new_heart.set_eaten(1);
            m_hearts_bar.emplace_back(new_heart);
        }
        else
            return false;
    }

    for (int j = m_hearts_grey_bar.size(); j < 5; j++)
    {
        Heart_Grey new_heart;
        if (new_heart.init())
        {
            new_heart.set_scale(vec2{0.08f, 0.08f});
            new_heart.set_position(vec2{20 + 45.0f * j, 20});
            m_hearts_grey_bar.emplace_back(new_heart);
        }
        else
            return false;
    }
    return true;
}

void World::init_level(int level_num)
{
    parse_sum_file(current_level);
    std::vector<Camera_View> new_camera_views;
    m_level = {0, 0, level_info[2], level_info[3], level_info[4], level_info[5],
               // current grid should be 0,0
               {0, 0},
               {level_info[0], level_info[1]},
               new_camera_views};

    if (level_num == 0)
        create_tutorial_level();
    else
    {

        std::vector<Camera_View> cvs(m_level.view_grid.col * m_level.view_grid.row);
        m_level.camera_views = cvs;
        // put key at the last camera view of every level
        m_level.key_view = m_level.view_grid.col * m_level.view_grid.row - 1;
        // set camera view of portal as the same as key,
        // spawns portal near the key
        m_level.portal_view = m_level.key_view;
        std::vector<int> empty_view(0);
        for (int i = 0; i < m_level.view_grid.row * m_level.view_grid.row; i++)
        {
            empty_view.push_back(i);
        }
        std::vector<int> filled_view(0);
        for (int i = 0; i < m_level.number_filled_view; i++)
        {
            int j = rand() % empty_view.size();
            filled_view.push_back(empty_view.at(j));
            empty_view.erase(empty_view.begin() + j);
        }
        for (int i : empty_view)
        {
            parse_view_file(i / m_level.view_grid.col, i % m_level.view_grid.col);
            std::vector<Star> m_stars = {};
            std::vector<Heart> m_hearts = {};
            std::vector<Enemy> m_enemies = {};
            Camera_View cv = {{{}, false},
                              {{}, false},
                              m_stars,
                              m_enemies,
                              m_hearts};
            m_level.camera_views[i] = cv;
            if (i == m_level.key_view)
                spawn_key();
        }
        for (int i : filled_view)
        {
            parse_view_file(i / m_level.view_grid.col, i % m_level.view_grid.col);
            Camera_View cv;
            m_level.camera_views[i] = cv;
            spawn_star(m_level.max_star, i);
            spawn_enemy(m_level.max_enemies, i);
            spawn_heart(m_level.max_heart, i);
            if (i == m_level.key_view)
                spawn_key();
        }
    }
}

void World::init_matrix()
{
    matrix_positions.clear();
    // the range of i is 0 to 12 but use 1 to 11 so that asset does appear on the margin
    // the range of j is 0 to 8 but use 1 to 7 so that asset does appear on the margin
    for (int i = 1; i < 11; i++)
    {
        for (int j = 1; j < 7; j++)
        {
            if ("G" == singleview[i][j])
            {
                // printf("%s\n", singleview[i][j].c_str());
                matrix_positions.emplace_back(grid[i][j]);
            }
        }
    }

    // not sure if this fixes the prince position issue
    matrix_positions.emplace_back(m_prince.get_position());
}

void World::create_tutorial_level()
{
    Key new_key;
    if (!new_key.init())
        return;
    Portal new_portal;
    if (!new_portal.init())
        return;
    vec2 center_pos = {600.f, 400.f};

    m_level.number_filled_view = 4;
    m_level.key_view = 3;
    m_level.portal_view = 3;
    m_level.max_star = 1;
    m_level.max_heart = 1;
    m_level.max_enemies = 1;
    m_level.view_grid.row = 1;
    m_level.view_grid.col = 4;

    // view 00
    std::vector<Star> m_stars00(0);
    Star s00;
    s00.init();
    s00.set_position(center_pos);
    m_stars00.emplace_back(s00);
    std::vector<Enemy> m_enemys00 = {};
    std::vector<Heart> m_hearts00 = {};
    Camera_View cv00 = {
        {{}, false},
        {{}, false},
        m_stars00,
        m_enemys00,
        m_hearts00};
    m_level.camera_views.emplace_back(cv00);

    // view 01
    std::vector<Star> m_stars01;
    Star s01;
    s01.init();
    s01.set_position(center_pos);
    m_stars01.emplace_back(s01);
    std::vector<Enemy> m_enemys01 = {};
    Enemy e01;
    Bullet b01;
    e01.init(current_level) & b01.init(0);
    e01.set_position(center_pos);
    e01.set_guard_position(center_pos);
    e01.set_key(enemy_count);
    m_enemys01.emplace_back(e01);
    b01.set_position(e01.get_position());
    m_bullets_enemy.emplace(enemy_count, b01);
    ++enemy_count;

    std::vector<Heart> m_hearts01 = {};
    Camera_View cv01 = {{{}, false},
                        {{}, false},
                        m_stars01,
                        m_enemys01,
                        m_hearts01};
    m_level.camera_views.emplace_back(cv01);

    // view 02
    std::vector<Star> m_stars02 = {};
    std::vector<Enemy> m_enemys02 = {};
    std::vector<Heart> m_hearts02 = {};
    Heart h02;
    h02.init();
    h02.set_position(center_pos);
    m_hearts02.emplace_back(h02);
    Camera_View cv02 = {{{}, false},
                        {{}, false},
                        m_stars02,
                        m_enemys02,
                        m_hearts02};
    m_level.camera_views.emplace_back(cv02);

    // view 03
    new_key.set_position(center_pos);
    new_portal.set_position(add(center_pos, {200.f, 200.f}));
    std::vector<Star> m_stars03 = {};
    std::vector<Enemy> m_enemys03 = {};
    std::vector<Heart> m_hearts03 = {};
    Camera_View cv03 = {{new_key, true},
                        {new_portal, true},
                        m_stars03,
                        m_enemys03,
                        m_hearts03};
    m_level.camera_views.emplace_back(cv03);
}

void World::clear_camera_view()
{
    int num_views = m_level.view_grid.col * m_level.view_grid.row;
    for (int i = 0; i < num_views; i++)
    {
        if (m_level.camera_views[i].m_opt_key.is_this_view)
            m_level.camera_views[i].m_opt_key.m_key.destroy();
        if (m_level.camera_views[i].m_opt_portal.is_this_view)
            m_level.camera_views[i].m_opt_portal.m_portal.destroy();
        for (auto &star : m_level.camera_views[i].m_stars)
            star.destroy();
        m_level.camera_views[i].m_stars.clear();
        for (auto &heart : m_level.camera_views[i].m_hearts)
            heart.destroy();
        m_level.camera_views[i].m_hearts.clear();
        for (auto &wall : m_walls)
            wall.destroy();
        m_walls.clear();
        for (auto &enemy : m_level.camera_views[i].m_enemies)
            enemy.destroy();
        m_level.camera_views[i].m_enemies.clear();
    }
    m_level.camera_views.clear();
    for (int i = 0; i < enemy_count; i++)
    {
        m_bullets_enemy[i].destroy();
        m_bullets_enemy.erase(i);
    }
    for (auto &bullet : m_bullets_prince_)
        bullet.destroy();
    m_bullets_prince_.clear();
}

void World::load_save_file()
{
    std::string file_path = data_path "/save.txt";
    std::string line;
    std::ifstream saveFile;

    saveFile.open(file_path);

    if (!saveFile.is_open())
    {
        fprintf(stderr, "Failed to load save file!\n");
    }

    std::vector<int> v;

    while (std::getline(saveFile, line))
    {

        std::string delimiter = " ";
        size_t pos = 0;
        std::string token;

        // Split the line into characters by using the " " delimiter
        while ((pos = line.find(delimiter)) != std::string::npos)
        {
            // Extract the substring based on the first found delimiter
            // and add to our matrix
            token = line.substr(0, pos);
            v.emplace_back(std::stoi(token));

            // Erase the current character and space from the line
            line.erase(0, pos + delimiter.length());
        }

        v.emplace_back(std::stoi(line));
    }

    // resets everything based on save file, might need to re-init prince
    matrix_positions.clear();
    Walls::set_texture_change();
    Enemy::set_texture_change();
    current_level = v[0];
    m_level.current_view.row = v[1];
    m_level.current_view.col = v[2];
    m_prince.m_points = v[3];

    m_prince.set_lives(v[4]);

    set_bars();

    // called so things can be placed
    init_level(current_level);
    // need to have something else here that changes the textures
    // of the enemy to match the current level
    Walls::set_texture_change();
    Enemy::set_texture_change();
    saveFile.close();
}

void World::write_to_save_file()
{
    std::string file_path = data_path "/save.txt";
    std::ofstream saveFile;
    std::string line;

    line += std::to_string(current_level) + " ";
    line += std::to_string(m_level.current_view.row) + " ";
    line += std::to_string(m_level.current_view.col) + " ";
    line += std::to_string(m_prince.m_points) + " ";
    line += std::to_string(m_prince.get_lives());

    saveFile.open(file_path);

    if (!saveFile.is_open())
    {
        fprintf(stderr, "Failed to save progress!\n");
    }

    saveFile << line;
    saveFile.close();
}
