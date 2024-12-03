#include "Game.h"
#include "GeneratorManager.h"
#include "CollisionManager.h"
#include "Player.h"
#include "Renderer.h"
#include "FallingItem.h"
#include "Animation.h"
#include "appConfjs.h"
#include "Constants.h"
#include "Assets.h"

//for debugging
void drawAABB(sf::RenderWindow& window, const sf::Vector2f& ul, const sf::Vector2f& lr) {
    sf::Vector2f size(lr.x - ul.x, lr.y - ul.y);

    sf::RectangleShape rectangle(size);
    rectangle.setPosition(ul);
    rectangle.setFillColor(sf::Color::Transparent);
    rectangle.setOutlineColor(sf::Color::Red);
    rectangle.setOutlineThickness(1.f);

    window.draw(rectangle);
}

bool Game::run(sf::RenderWindow* win)
{
    init(win);
    loop(win);
    end(win);
    return false;
}

void Game::init(sf::RenderWindow* win)
{
    win->setTitle("Game");
}

void Game::loop(sf::RenderWindow* win)
{
    sf::View camera(sf::FloatRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT));

    srand(time(0));
    Generator generator(FOOD_COOLDOWN);
    CollisionManager collisionMngr(&generator.items);
    Renderer renderer(&generator.items);

    Player player;
    sf::Texture idle;
    if (!idle.loadFromFile(animationSheet_path)) {
        printf("can not load animation sheet");
    }
    player.frame = sf::RectangleShape(sf::Vector2f(idle.getSize().x / 4, idle.getSize().y / 1));
    player.frame.setTexture(&idle);

    player.frame.setOrigin(player.frame.getSize().x, player.frame.getSize().y);
    player.frame.setPosition(WINDOW_WIDTH / 2 - PLAYER_W / 2, PLATFORM_POSITION);

    player.collider.initCollider
    (
        Vector2d(300, 450),
        Vector2d(300 + PLAYER_W * 2.5, 450 + PLAYER_H * 2.5)
    );

    player.collider.position = Vector2d(player.frame.getPosition().x, player.frame.getPosition().y);
    //player.updateCollider(player.frame.getPosition());

    Animation idleANim(&idle, sf::Vector2u(4, 1), 0.2 * 1.5);
    float deltaTime;
    sf::Clock clock;
    sf::Texture BG;
    if (!BG.loadFromFile("./resources/bg.png")) {
        printf("can not load bg.png");
    }
    sf::Sprite backGround(BG);
    //backGround.setOrigin(0, 0);
    sf::Texture spritesheet;
    if (!spritesheet.loadFromFile(spritesheet_path)) {
        printf("can not load sprites.png");
    }

    sf::Shader shader;
    if (!shader.loadFromFile("trippy_shader.frag", sf::Shader::Fragment)) {
        printf("can not load trippy_shader.frag");
    }

    // Text
    sf::Font scoreFont;
    if (!scoreFont.loadFromFile("./resources/Fonts/KarmaticArcade.ttf")) {
        std::cerr << "Error: Could not load font!" << std::endl;
    }

    sf::Text scoreText;
    scoreText.setFont(scoreFont);
    scoreText.setCharacterSize(32);
    scoreText.setFillColor(sf::Color::Black);
    scoreText.setOrigin(scoreText.getGlobalBounds().width / 2.f, scoreText.getGlobalBounds().height / 2.f);
    //scoreText.setOrigin(WINDOW_WIDTH / 2, 0);

    float backgroundOffsetX = 0.f;
    float backgroundWidth = static_cast<float>(BG.getSize().x);

    while (win->isOpen())
    {
        // Check for all window events
        sf::Event event;
        while (win->pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                win->close();
            else if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Escape)
                {
                    flag = !flag;
                    win->setMouseCursorVisible(flag);
                    win->setMouseCursorGrabbed(!flag);
                }
            }
        }
        generator.generate(&spritesheet);
        shader.setUniform("time", clock.getElapsedTime().asSeconds());

        scoreText.setString("Score: " + std::to_string(player.score));

        player.handleInput();
        idleANim.Update(0, 1.0f / FPS);
        player.frame.setTextureRect(idleANim.uvRect);
        collisionMngr.Update(1 / FPS);
        switch (collisionMngr.checkCollisionWith(&player.collider))
        {
        case healthy:
            player.score += 10;
            break;
        case junk:
            player.health--;
            player.isTripped = true;
            break;
        }
        if (player.health < 1)
            break;

        float playerX = player.frame.getPosition().x;
        playerX = std::max(0.f, std::min(playerX, BG_W - player.frame.getSize().x));

        float cameraX = std::max(WINDOW_WIDTH / 2.0f, std::min(playerX + 16, backgroundWidth - WINDOW_WIDTH / 2.0f));
        scoreText.setPosition(cameraX, 20.f);
        camera.setCenter(cameraX, WINDOW_HEIGHT / 2.0f);

        win->setView(camera);
        win->clear();
        //backGround.setPosition(-backgroundOffsetX, 0.f);
        if (player.isTripped)
            win->draw(backGround, &shader);
        else
            win->draw(backGround);
        renderer.Render(win);
        win->draw(scoreText);

        //to be deleted
        drawAABB(*win, player.collider.ul, player.collider.lr);
        win->draw(player.frame);
        win->display();
    }
}

void Game::end(sf::RenderWindow* win)
{

}

void Game::draw(sf::RenderWindow* win)
{
}

