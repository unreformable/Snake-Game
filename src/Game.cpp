#include "Game.hpp"
#include "Utilities.cpp"



Game::Game(int screen_w, int screen_h, const char* title):
snake(Vec2i{4,4}), pellet(this)
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(screen_w, screen_h, 0, &window, &renderer);
    SDL_SetWindowTitle(window, title);
}

Game::~Game()
{
    if(renderer)
        SDL_DestroyRenderer(renderer);
    if(window)
        SDL_DestroyWindow(window);
    SDL_Quit();
}

void Game::Run()
{
    constexpr uint32_t move_delay_ms = 150;
    uint32_t start_ms = SDL_GetTicks();
    Direction next_move_dir = Direction::NONE;
    SDL_Event e;

    while(!quit)
    {
        if(SDL_TICKS_PASSED(SDL_GetTicks() - move_delay_ms, start_ms))
        {
            start_ms = SDL_GetTicks();

            if(are_opposite(snake.GetMoveDir(), next_move_dir))
                next_move_dir = snake.GetMoveDir();
            
            // Check collision with tail before movement
            const Vec2i next_snake_pos = get_position_in_front(snake.GetHeadPos(), next_move_dir);
            if(next_snake_pos != snake.GetBackPos() && snake.IsCollision(next_snake_pos))
            {
                GameOver();
                return;
            }

            snake.Move(next_move_dir);
            if(snake.GetHeadPos() == pellet.GetPos())
            {
                snake.Grow();
                if(snake.GetLength() == 16*9)
                {
                    // TODO: Make win animation
                }
                else
                    pellet.Reposition();
            }
            Render();
        }

        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT)
            {
                quit = true;
            }
            else if(e.type == SDL_KEYDOWN)
            {
                switch(e.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        quit = true;
                        break;
                    case SDLK_q:
                        quit = true;
                        break;
                    case SDLK_UP:
                        next_move_dir = Direction::UP;
                        break;
                    case SDLK_DOWN:
                        next_move_dir = Direction::DOWN;
                        break;
                    case SDLK_LEFT:
                        next_move_dir = Direction::LEFT;
                        break;
                    case SDLK_RIGHT:
                        next_move_dir = Direction::RIGHT;
                        break;
                }
            }
        }
    }
}

void Game::GameOver()
{
    constexpr int anim_time_ms = 2000;
    const int delay_ms = anim_time_ms/snake.GetLength();
    uint32_t start_ms = SDL_GetTicks();

    while(snake.GetLength() > 0)
    {
        if(SDL_TICKS_PASSED(SDL_GetTicks() - delay_ms, start_ms))
        {
            start_ms = SDL_GetTicks();

            snake.SetLength(snake.GetLength() - 1);
            Render();
        }
    }
}

const std::unique_ptr<Vec2i[]> Game::GetUnoccupiedPositions(int& count) const
{
    // Find all occupied positions
    bool occupied[16*9];

    Vec2i body_pos;
    for(int i = 0; i < snake.GetLength(); i++)
    {
        body_pos = snake.GetPositions()[i];

        occupied[body_pos.y*16 + body_pos.x] = true;
    }
    occupied[pellet.GetPos().y*16, pellet.GetPos().x] = true;

    // Count unoccupied positions
    count = 0;
    for(int i = 0; i < 16*9; i++)
        if(occupied[i] == false)
            count++;
    
    // Create array for unoccupied positions
    std::unique_ptr<Vec2i[]> unoccupied = std::make_unique<Vec2i[]>(16*9);
    int i = 0;
    for(int y = 0; y < 9; y++)
    {
        for(int x = 0; x < 16; x++)
        {
            if(occupied[y*16 + x] == false)
            {
                unoccupied[i] = Vec2i{x,y};
                i++;
            }
        }
    }

    return unoccupied;
}

bool Game::IsOccupied(const Vec2i& pos) const
{
    for(int i = 0; i < snake.GetLength(); i++)
        if(snake.GetPositions()[i] == pos)
            return true;
    if(pellet.GetPos() == pos)
        return true;
    return false;
}

void Game::Render()
{
    // Draw background
    SDL_SetRenderDrawColor(renderer, 120, 120, 120, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    // Make mesh-like structure
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_Rect rect{0, 0, 50, 50};
    for(int y = 0; y < 9; y++)
    {
        for(int x = 0; x < 16; x++)
        {
            rect.x = x * 50;
            rect.y = y * 50;
            SDL_RenderDrawRect(renderer, &rect);
        }
    }

    // Render all actors
    pellet.Draw(renderer);
    snake.Draw(renderer);

    // Display
    SDL_RenderPresent(renderer);
}