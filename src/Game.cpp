#include <SFML/Graphics.hpp>
#include <memory>

#include <iostream> 
#include "Snake.cpp"
#include "Food.cpp"

namespace {
constexpr unsigned int WINDOW_WIDTH = 800;
constexpr unsigned int WINDOW_HEIGHT = 600;
constexpr float GRID_CELL_SIZE = 20.0f;
constexpr float MOVE_INTERVAL_SECONDS = 0.12f;

bool isOpposite(const sf::Vector2f& lhs, const sf::Vector2f& rhs) {
    return lhs.x == -rhs.x && lhs.y == -rhs.y;
}
}

class Game {
public:
    Game();
    void run();

private:
    void processEvents();
    void handleKeyPress(sf::Keyboard::Key key);
    void update();
    void handleCollisions();
    void render();

    bool isOutOfBounds(const sf::Vector2f& position) const;
    void reset();

    sf::RenderWindow m_window;
    Snake m_snake;
    Food m_food;
    sf::Clock m_moveClock;
    float m_moveInterval;
    sf::Vector2f m_direction;
    bool m_gameOver;
    int m_score = 0;
    sf::Font m_font;
    std::unique_ptr<sf::Text> m_gameOverText;
    std::unique_ptr<sf::Text> m_scoreText;

};

Game::Game()
    : m_window(sf::VideoMode(sf::Vector2u(WINDOW_WIDTH, WINDOW_HEIGHT)), "Snake Game"),
      m_snake(),
      m_food(),
      m_moveInterval(MOVE_INTERVAL_SECONDS),
      m_direction(GRID_CELL_SIZE, 0.0f),
      m_gameOver(false) {
    m_window.setFramerateLimit(60);
    m_food.spawn(m_window.getSize());
    m_snake.setDirection(m_direction);
    m_score = 0; 

    // Load font and setup game over text
    if (!m_font.openFromFile("arial.ttf")) {
        std::cerr << "Error loading font" << std::endl;
    }
    m_gameOverText = std::make_unique<sf::Text>();
    m_gameOverText->setFont(m_font);
    m_gameOverText->setString("Game Over\nPress SPACE to Restart");
    m_gameOverText->setCharacterSize(32);
    m_gameOverText->setFillColor(sf::Color::White);
    m_gameOverText->setPosition(sf::Vector2f(200.f, 250.f));
    
    // Setup score text
    m_scoreText = std::make_unique<sf::Text>();
    m_scoreText->setFont(m_font);
    m_scoreText->setCharacterSize(24);
    m_scoreText->setFillColor(sf::Color::Yellow);
    m_scoreText->setPosition(sf::Vector2f(10.f, 10.f));
    m_scoreText->setString("Score: 0");
}

void Game::run() {
    while (m_window.isOpen()) {
        processEvents();
        update();
        render();
    }
}

void Game::processEvents() {
    while (const std::optional<sf::Event> event = m_window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            m_window.close();
        } else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            handleKeyPress(keyPressed->code);
        }
    }
}

void Game::handleKeyPress(sf::Keyboard::Key key) {
    if (key == sf::Keyboard::Key::Escape) {
        m_window.close();
        return;
    }

    if (key == sf::Keyboard::Key::Space && m_gameOver) {
        reset();
        return;
    }

    if (m_gameOver) {
        return;
    }

    sf::Vector2f requested = m_direction;
    switch (key) {
        case sf::Keyboard::Key::Up:
            requested = sf::Vector2f(0.0f, -GRID_CELL_SIZE);
            break;
        case sf::Keyboard::Key::Down:
            requested = sf::Vector2f(0.0f, GRID_CELL_SIZE);
            break;
        case sf::Keyboard::Key::Left:
            requested = sf::Vector2f(-GRID_CELL_SIZE, 0.0f);
            break;
        case sf::Keyboard::Key::Right:
            requested = sf::Vector2f(GRID_CELL_SIZE, 0.0f);
            break;
        default:
            return;
    }

    if (!isOpposite(requested, m_direction)) {
        m_direction = requested;
        m_snake.setDirection(m_direction);
    }
}

void Game::update() {
    if (m_gameOver) {
        return;
    }

    if (m_moveClock.getElapsedTime().asSeconds() >= m_moveInterval) {
        m_snake.move();
        m_moveClock.restart();
        handleCollisions();
    }
}

void Game::handleCollisions() {
    const sf::Vector2f headPos = m_snake.getHead().getPosition();

    if (isOutOfBounds(headPos)) {
        m_gameOver = true;
        return;
    }

    const auto& segments = m_snake.getBody();
    for (std::size_t i = 1; i < segments.size(); ++i) {
        if (segments[i].getPosition() == headPos) {
            m_gameOver = true;
            return;
        }
    }

    if (headPos == m_food.getShape().getPosition()) {
        m_snake.grow();
        m_food.spawn(m_window.getSize());
        m_score += 10; 
        m_scoreText->setString("Score: " + std::to_string(m_score));
        std::cout << "Score: " << m_score << std::endl;
    }
}

void Game::render() {
    m_window.clear(sf::Color::Black);

    for (const auto& segment : m_snake.getBody()) {
        m_window.draw(segment);
    }
    
    if (m_gameOver) {
    m_window.draw(*m_gameOverText);
    }
    
    m_window.draw(m_food.getShape());

    m_window.draw(*m_scoreText);

    m_window.display();
}

bool Game::isOutOfBounds(const sf::Vector2f& position) const {
    return position.x < 0.0f || position.y < 0.0f ||
           position.x >= static_cast<float>(WINDOW_WIDTH) ||
           position.y >= static_cast<float>(WINDOW_HEIGHT);
}

void Game::reset() {
    m_snake = Snake();
    m_food.spawn(m_window.getSize());
    m_direction = sf::Vector2f(GRID_CELL_SIZE, 0.0f);
    m_snake.setDirection(m_direction);
    m_moveClock.restart();
    m_gameOver = false;
    m_score = 0;
}
