#!/usr/bin/env python3
import argparse
import json
import os
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
import subprocess
import sys
import time
from urllib.parse import urlparse

ROOT = Path(__file__).resolve().parent
DEFAULT_FRONTEND_DIR = ROOT / "frontend"
DEFAULT_BACKEND_BIN = ROOT / "build" / "backend" / "railway_navigator"


def try_patch_input(text: str) -> str:
    tokens = text.split()
    if not tokens:
        return text

    idx = 0

    def read_number():
        nonlocal idx
        if idx >= len(tokens):
            return None
        try:
            value = float(tokens[idx])
        except ValueError:
            return None
        idx += 1
        return value

    def read_int():
        value = read_number()
        if value is None:
            return None
        if value != int(value):
            return None
        return int(value)

    n = read_int()
    m = read_int()
    if n is None or m is None or n <= 0 or m < 0:
        return text

    for _ in range(3):
        if read_number() is None:
            return text

    for _ in range(9):
        if read_number() is None:
            return text

    for _ in range(n):
        if read_number() is None:
            return text

    for _ in range(m):
        if read_int() is None:
            return text
        if read_int() is None:
            return text
        if read_int() is None:
            return text
        if read_number() is None:
            return text
        if read_number() is None:
            return text

    if idx < len(tokens):
        return text

    if text.endswith("\n"):
        return f"{text}0\n"
    return f"{text}\n0\n"


class RailwayServer(ThreadingHTTPServer):
    def __init__(self, server_address, handler_cls, backend_path, timeout):
        super().__init__(server_address, handler_cls)
        self.backend_path = Path(backend_path)
        self.backend_timeout = timeout


class Handler(SimpleHTTPRequestHandler):
    server_version = "RailwayNavigatorHTTP/1.0"

    def do_OPTIONS(self):
        if urlparse(self.path).path.rstrip("/") == "/api/run":
            self.send_response(204)
            self.send_header("Access-Control-Allow-Origin", "*")
            self.send_header("Access-Control-Allow-Methods", "POST, OPTIONS")
            self.send_header("Access-Control-Allow-Headers", "Content-Type")
            self.end_headers()
            return
        self.send_error(404, "Not Found")

    def do_POST(self):
        if urlparse(self.path).path.rstrip("/") != "/api/run":
            self.send_error(404, "Not Found")
            return

        length = int(self.headers.get("Content-Length", "0"))
        raw = self.rfile.read(length)
        content_type = self.headers.get("Content-Type", "")

        input_text = ""
        if raw:
            if "application/json" in content_type:
                try:
                    payload = json.loads(raw.decode("utf-8"))
                except json.JSONDecodeError:
                    self.send_json(400, {"ok": False, "error": "invalid json"})
                    return
                input_text = payload.get("input", "") if isinstance(payload, dict) else ""
            else:
                input_text = raw.decode("utf-8", errors="replace")

        if not isinstance(input_text, str):
            self.send_json(400, {"ok": False, "error": "input must be a string"})
            return

        input_text = try_patch_input(input_text)

        backend_path = self.server.backend_path
        if not backend_path.exists():
            self.send_json(
                200,
                {
                    "ok": False,
                    "error": "backend binary not found",
                    "path": str(backend_path),
                },
            )
            return

        start = time.time()
        try:
            result = subprocess.run(
                [str(backend_path)],
                input=input_text,
                text=True,
                capture_output=True,
                cwd=str(ROOT),
                timeout=self.server.backend_timeout,
            )
        except subprocess.TimeoutExpired:
            self.send_json(200, {"ok": False, "error": "backend timeout"})
            return
        except OSError as exc:
            self.send_json(200, {"ok": False, "error": f"backend error: {exc}"})
            return

        duration_ms = int((time.time() - start) * 1000)
        payload = {
            "ok": result.returncode == 0,
            "exit_code": result.returncode,
            "stdout": result.stdout,
            "stderr": result.stderr,
            "duration_ms": duration_ms,
        }
        self.send_json(200, payload)

    def send_json(self, status, payload):
        data = json.dumps(payload, ensure_ascii=False).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(data)))
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(data)


def build_arg_parser():
    parser = argparse.ArgumentParser(description="Local server for Railway Navigator")
    parser.add_argument("--host", default="127.0.0.1", help="bind host")
    parser.add_argument("--port", type=int, default=8000, help="bind port")
    parser.add_argument(
        "--frontend",
        default=str(DEFAULT_FRONTEND_DIR),
        help="path to frontend directory",
    )
    parser.add_argument(
        "--backend",
        default=os.environ.get("BACKEND_BIN", str(DEFAULT_BACKEND_BIN)),
        help="path to backend binary",
    )
    parser.add_argument(
        "--timeout",
        type=int,
        default=15,
        help="backend timeout in seconds",
    )
    return parser


def main():
    parser = build_arg_parser()
    args = parser.parse_args()

    frontend_dir = Path(args.frontend).resolve()
    backend_path = Path(args.backend).resolve()

    if not frontend_dir.exists():
        print(f"Frontend directory not found: {frontend_dir}", file=sys.stderr)
        return 2

    if not backend_path.exists():
        print(
            f"Warning: backend binary not found: {backend_path}", file=sys.stderr
        )

    handler = lambda *handler_args, **handler_kwargs: Handler(
        *handler_args, directory=str(frontend_dir), **handler_kwargs
    )

    server = RailwayServer((args.host, args.port), handler, backend_path, args.timeout)
    url = f"http://{args.host}:{args.port}/"
    print(f"Serving frontend from {frontend_dir}")
    print(f"Backend binary: {backend_path}")
    print(f"Open: {url}")

    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nShutting down...")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
