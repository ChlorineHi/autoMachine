from flask import Flask, render_template, request, send_from_directory, jsonify
import subprocess
import os

app = Flask(__name__)

@app.route('/')
def index():
    return send_from_directory('.', 'index.html')

@app.route('/process_regex')
def process_regex():
    regex = request.args.get('regex', '')
    if not regex:
        return jsonify({"error": "请输入正则表达式"}), 400

    try:
        # 执行 regex_cli.exe 并获取输出
        result = subprocess.run(
            ['regex_cli.exe', regex], 
            capture_output=True, 
            text=True, 
            cwd=os.path.dirname(os.path.abspath(__file__))
        )
        
        if result.returncode == 0:
            return jsonify({"result": result.stdout})
        else:
            return jsonify({"error": f"执行错误: {result.stderr}"}), 500
    except Exception as e:
        return jsonify({"error": f"处理异常: {str(e)}"}), 500

if __name__ == '__main__':
    app.run(debug=True, port=5000)
