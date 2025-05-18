import json
import subprocess
import time
from django.http import JsonResponse
from django.views.decorators.csrf import csrf_exempt

@csrf_exempt
def analyze_code(request):
    if request.method == 'POST':
        try:
            data = json.loads(request.body)
            code = data.get('code')
            language = data.get('language')

            if not code or not language:
                return JsonResponse({'error': 'Missing code or language'}, status=400)

            # Write the code to the file that rec.cpp will use
            with open("C:\\Users\\abhin\\TimeComplexity\\code.txt", "w") as file:
                file.write(code)

            cpp_path = "C:\\Users\\abhin\\TimeComplexity\\rec.cpp"
            exe_path = "C:\\Users\\abhin\\TimeComplexity\\rec.exe"
            result_path = "C:\\Users\\abhin\\TimeComplexity\\result.txt"

            # Compile the C++ code
            # subprocess.run(
            #     ['g++', cpp_path, '-o', exe_path],
            #     capture_output=True, text=True, check=True
            # )

            # Run the compiled executable
            subprocess.run(
                [exe_path],
                capture_output=True, text=True, check=True
            )

            # ⏳ Optional: Add buffer time to ensure file write is completed
            time.sleep(1)  # Usually sufficient; change to 15 if needed

            # Read the result from file
            with open(result_path, "r") as result_file:
                time_complexity = result_file.read().strip()

            return JsonResponse({'time_complexity': time_complexity}, status=200)

        except subprocess.CalledProcessError as err:
            return JsonResponse({'error': 'Error compiling or running C++ code', 'details': err.stderr}, status=500)

        except json.JSONDecodeError:
            return JsonResponse({'error': 'Invalid JSON format'}, status=400)

        except FileNotFoundError:
            return JsonResponse({'error': 'Result file not found'}, status=500)

    return JsonResponse({'error': 'Invalid request method'}, status=400)
