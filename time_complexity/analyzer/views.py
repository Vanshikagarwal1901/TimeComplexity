import json
from django.http import JsonResponse
from django.views.decorators.csrf import csrf_exempt

@csrf_exempt
def analyze_code(request):
    if request.method == 'POST':
        try:
            data = json.loads(request.body)  # Parse JSON data
            code = data.get('code')
            language = data.get('language')

            if not code or not language:
                return JsonResponse({'error': 'Missing code or language'}, status=400)

            # **Write Code to a File**
            with open("C:\\Users\\Hp\\OneDrive\\Desktop\\TimeComplexity\\code.txt", "w") as file:
                file.write(code)

            # Placeholder for analyzing time complexity
            time_complexity = "O(n)"  # Example response

            return JsonResponse({'time_complexity': time_complexity}, status=200)

        except json.JSONDecodeError:
            return JsonResponse({'error': 'Invalid JSON format'}, status=400)

    return JsonResponse({'error': 'Invalid request method'}, status=400)