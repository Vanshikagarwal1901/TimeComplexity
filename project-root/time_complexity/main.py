from fastapi import FastAPI, Request
from fastapi.middleware.cors import CORSMiddleware
import subprocess
import os

app = FastAPI()


app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"], 
    allow_methods=["*"],
    allow_headers=["*"],
)

@app.post("/analyze_code/")
async def analyze_code(request: Request):
    data = await request.json()
    code = data.get("code")

    try:
        project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))

       
        code_txt_path = os.path.join(project_root, "code.txt")
        with open(code_txt_path, "w") as f:
            f.write(code)

        
        exe_path = os.path.join(project_root, "rec.exe")

        
        result = subprocess.run(
            [exe_path],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            timeout=10,
            shell=True  
        )


        result_txt_path = os.path.join(project_root, "result.txt")
        if os.path.exists(result_txt_path):
            with open(result_txt_path, "r") as f:
                output = f.read().strip()
        else:
            output = result.stdout.strip()  

        return {"time_complexity": output}

    except Exception as e:
        return {"error": str(e)}
