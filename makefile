# Define variables for directories and commands
WWW_DIR := www
DOCKER_COMPOSE := docker-compose
YARN := yarn

# Phony targets are not actual files
.PHONY: all build up clean install help

# Default target executed when running `make` without arguments
all: build up

# Build the www directory using Yarn
build:
	@echo "Building the www directory..."
	cd $(WWW_DIR) && $(YARN) install
	cd $(WWW_DIR) && $(YARN) build
	@echo "Build completed."

# Start Docker containers using docker-compose
up: build
	@echo "Starting Docker containers..."
	$(DOCKER_COMPOSE) up --build
	@echo "Docker containers are up."

# Stop Docker containers
down:
	@echo "Stopping Docker containers..."
	$(DOCKER_COMPOSE) down
	@echo "Docker containers are down."

# Remove build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(WWW_DIR)/dist
	@echo "Cleaned."

# Install dependencies without building
install:
	@echo "Installing dependencies..."
	cd $(WWW_DIR) && $(YARN) install
	@echo "Dependencies installed."
