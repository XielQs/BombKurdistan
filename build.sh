#!/bin/bash

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # no color

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

show_help() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -h, --help          Show this help message"
    echo "  -d, --debug         Build in debug mode"
    echo "  -c, --clean         Clean build directory before building"
    echo "  -j, --jobs N        Use N parallel jobs (default: CPU cores)"
    echo "  -z, --zip           Create ZIP packages after build"
    echo "  -w, --windows       Cross-compile for Windows"
    echo "  --no-discord        Build without Discord RPC support"
}

BUILD_TYPE="Release"
CLEAN_BUILD=false
JOBS=$(nproc 2>/dev/null || echo 1)
CREATE_ZIP=false
WINDOWS_BUILD=false
DISCORD_SUPPORT=true
BUILD_DIR="build"

# arg parsing
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        -j|--jobs)
            JOBS="$2"
            shift 2
            ;;
        -z|--zip)
            CREATE_ZIP=true
            shift
            ;;
        -w|--windows)
            WINDOWS_BUILD=true
            BUILD_DIR="build_windows"
            DISCORD_SUPPORT=false
            shift
            ;;
        --no-discord)
            DISCORD_SUPPORT=false
            shift
            ;;
        *)
            log_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

is_command_available() {
    command -v "$1" >/dev/null 2>&1
}

check_dependencies() {
    local missing_deps=()

    if ! is_command_available cmake; then
        missing_deps+=("cmake")
    fi

    if [[ "$WINDOWS_BUILD" == true ]] && ! is_command_available x86_64-w64-mingw32-g++; then
        missing_deps+=("mingw-w64")
    fi

    if [[ "$CREATE_ZIP" == true ]] && ! is_command_available zip; then
        missing_deps+=("zip")
    fi

    if [[ ${#missing_deps[@]} -gt 0 ]]; then
        log_error "Missing dependencies: ${missing_deps[*]}"
        log_info "Please install them and try again."
        exit 1
    fi
}

build_project() {
    log_info "Starting build process..."
    log_info "Build type: $BUILD_TYPE"
    log_info "Jobs: $JOBS"
    log_info "Windows build: $WINDOWS_BUILD"
    log_info "Discord support: $DISCORD_SUPPORT$( [[ "$WINDOWS_BUILD" == true ]] && echo " (not supported on windows)" )"

    if [[ "$CLEAN_BUILD" == true ]]; then
        log_info "Cleaning build directory..."
        rm -rf "$BUILD_DIR"
    fi

    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    local cmake_args=()
    cmake_args+=("-DCMAKE_BUILD_TYPE=$BUILD_TYPE")

    if [[ "$BUILD_TYPE" == "Debug" ]]; then
        cmake_args+=("-DDEBUG_MODE=ON")
    fi

    if [[ "$DISCORD_SUPPORT" == false ]]; then
        cmake_args+=("-DDISCORD_RPC=OFF")
    else
        cmake_args+=("-DDISCORD_RPC=ON")
    fi

    if [[ "$WINDOWS_BUILD" == true ]]; then
        cmake_args+=("-DCMAKE_TOOLCHAIN_FILE=../cmake/mingw-toolchain.cmake")
    fi

    log_info "Running cmake configure..."
    cmake "${cmake_args[@]}" ..

    # Build
    log_info "Building project..."
    cmake --build . --config "$BUILD_TYPE" --parallel "$JOBS"

    cd ..

    log_success "Build completed successfully!"

    if [[ "$WINDOWS_BUILD" == true ]]; then
        if [[ -f "bombkurdistan.exe" ]]; then
            log_success "Windows binary created: bombkurdistan.exe"
        fi
    else
        if [[ -f "bombkurdistan" ]]; then
            log_success "Linux binary created: bombkurdistan"
            # strip the binary
            if is_command_available strip; then
                log_info "Stripping binary..."
                strip bombkurdistan || log_error "Failed to strip binary"
            else
                log_info "strip command not found, skipping binary stripping."
            fi
        fi
    fi
}

# zipping
zip_packages() {
    if [[ "$CREATE_ZIP" == false ]]; then
        return
    fi

    log_info "Creating ZIP packages..."
    cd "$BUILD_DIR"

    cmake --build . --target zip

    cd ..
    log_success "ZIP packages created!"
}

main() {
    check_dependencies
    build_project
    zip_packages

    log_success "All done!"

    # binary info
    if [[ -f "bombkurdistan" ]]; then
        local size=$(stat -c%s "bombkurdistan" 2>/dev/null || stat -f%z "bombkurdistan" 2>/dev/null || echo "unknown")
        log_info "Linux binary size: $size bytes"
    fi

    if [[ -f "bombkurdistan.exe" ]]; then
        local size=$(stat -c%s "bombkurdistan.exe" 2>/dev/null || stat -f%z "bombkurdistan.exe" 2>/dev/null || echo "unknown")
        log_info "Windows binary size: $size bytes"
    fi
}

main "$@"
