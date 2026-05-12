import json
import shutil
import subprocess
from pathlib import Path
from django.http import JsonResponse
from django.views.decorators.csrf import csrf_exempt

PROJECT_ROOT = Path(__file__).resolve().parents[2]
ANALYZER_PROJECT = PROJECT_ROOT / "analyzer-csharp" / "ComplexityAnalyzer" / "ComplexityAnalyzer.csproj"
LOCAL_DOTNET = PROJECT_ROOT / ".tools" / "dotnet" / "dotnet"

@csrf_exempt
def analyze_code(request):
    if request.method == 'POST':
        try:
            data = json.loads(request.body)
            code = data.get('code')
            language = data.get('language')

            if not code or not language:
                return JsonResponse({'error': 'Missing code or language'}, status=400)

            if language != "C#":
                return JsonResponse(
                    {'error': 'This version analyzes C# code through the Roslyn analyzer.'},
                    status=400
                )

            dotnet = str(LOCAL_DOTNET) if LOCAL_DOTNET.exists() else shutil.which('dotnet')
            if dotnet is None:
                return JsonResponse(
                    {
                        'error': 'The .NET SDK is not installed or dotnet is not on PATH.',
                        'details': 'Install .NET 8+ to run the Roslyn C# analyzer.'
                    },
                    status=500
                )

            completed = subprocess.run(
                [dotnet, 'run', '--project', str(ANALYZER_PROJECT), '--no-launch-profile'],
                input=code,
                capture_output=True,
                text=True,
                timeout=30,
                check=True
            )

            payload = json.loads(completed.stdout)
            if payload.get('Errors'):
                return JsonResponse({'error': payload['Errors'][0], 'details': payload['Errors']}, status=400)

            return JsonResponse({
                'time_complexity': payload.get('TimeComplexity'),
                'nodes': payload.get('Nodes', []),
                'explanation': payload.get('Explanation', []),
            }, status=200)

        except FileNotFoundError:
            return JsonResponse({'error': 'The C# analyzer project could not be found.'}, status=500)
        except subprocess.CalledProcessError as err:
            return JsonResponse({'error': 'Error running the C# analyzer', 'details': err.stderr}, status=500)
        except subprocess.TimeoutExpired:
            return JsonResponse({'error': 'The C# analyzer timed out'}, status=500)

        except json.JSONDecodeError:
            return JsonResponse({'error': 'Invalid JSON format'}, status=400)

    return JsonResponse({'error': 'Invalid request method'}, status=400)
