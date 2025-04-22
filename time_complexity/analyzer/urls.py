from django.urls import path
from . import views

urlpatterns = [
    path('analyze_code/', views.analyze_code, name='analyze_code'),
]
