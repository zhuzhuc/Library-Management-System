-- Schema for library system
CREATE DATABASE IF NOT EXISTS library_system DEFAULT CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE library_system;

CREATE TABLE IF NOT EXISTS books (
  id INT PRIMARY KEY,
  title TEXT,
  author TEXT,
  isbn VARCHAR(64),
  category VARCHAR(128),
  total INT,
  available INT,
  INDEX idx_category (category),
  INDEX idx_author (author(100))
);

CREATE TABLE IF NOT EXISTS borrowers (
  id VARCHAR(64) PRIMARY KEY,
  type VARCHAR(32),
  name TEXT,
  department VARCHAR(128),
  max_limit INT,
  extra TEXT
);

-- Users table for login authentication
CREATE TABLE IF NOT EXISTS users (
  username VARCHAR(64) PRIMARY KEY,
  password VARCHAR(255) NOT NULL,
  user_type VARCHAR(32) NOT NULL,  -- 'admin' or 'user'
  borrower_id VARCHAR(64),  -- Links to borrowers table if regular user
  created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
  updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  INDEX idx_user_type (user_type),
  INDEX idx_borrower_id (borrower_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- Insert default admin user
INSERT IGNORE INTO users (username, password, user_type) VALUES 
('admin', 'admin123', 'admin'),
('user', 'user123', 'user');

-- Borrow records table for tracking borrow history
CREATE TABLE IF NOT EXISTS borrow_records (
  id INT AUTO_INCREMENT PRIMARY KEY,
  borrower_id VARCHAR(64),
  book_id INT,
  borrow_date DATETIME DEFAULT CURRENT_TIMESTAMP,
  return_date DATETIME NULL,
  FOREIGN KEY (borrower_id) REFERENCES borrowers(id) ON DELETE CASCADE,
  FOREIGN KEY (book_id) REFERENCES books(id) ON DELETE CASCADE,
  INDEX idx_borrower (borrower_id),
  INDEX idx_book (book_id),
  INDEX idx_borrow_date (borrow_date)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
