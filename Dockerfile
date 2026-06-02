# ==========================================
# Stage 1: Build C++ Core Retrieval Engine
# ==========================================
FROM python:3.11-slim AS cpp-builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY Makefile ./
COPY include/ ./include/
COPY src/ ./src/

# Compile C++ binaries
RUN make clean && make

# ==========================================
# Stage 2: Build React Frontend App
# ==========================================
FROM node:20-slim AS node-builder

WORKDIR /app

COPY frontend/package*.json ./
RUN npm ci

COPY frontend/ ./

# Pass build arguments. VITE_API_BASE_URL is set to empty string for unified port serving.
ARG VITE_API_BASE_URL=""
ENV VITE_API_BASE_URL=$VITE_API_BASE_URL

# Build client bundle
RUN npm run build

# ==========================================
# Stage 3: Production Runtime
# ==========================================
FROM python:3.11-slim AS runtime

# Install basic diagnostic tools
RUN apt-get update && apt-get install -y --no-install-recommends \
    curl \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy Python backend dependencies
COPY backend/requirements.txt ./backend/
RUN pip install --no-cache-dir -r backend/requirements.txt

# Copy compiled C++ binaries from cpp-builder
COPY --from=cpp-builder /app/search_engine /app/search_engine
COPY --from=cpp-builder /app/search_engine_api /app/search_engine_api

# Copy backend files
COPY backend/ ./backend/

# Copy default dataset collections
COPY datasets/ ./datasets/

# Copy documentation examples
COPY docs/ ./docs/

# Copy React production build from node-builder to the mounting target
COPY --from=node-builder /app/dist ./frontend/dist

# Expose API and frontend single-port
EXPOSE 8000

# Environment configurations
ENV INTELLISEARCH_CORE_BINARY=/app/search_engine_api
ENV INTELLISEARCH_DATASET=/app/datasets/smallDataset.txt
ENV PORT=8000

# Command to execute backend
CMD ["sh", "-c", "uvicorn backend.main:app --host 0.0.0.0 --port ${PORT:-8000}"]
