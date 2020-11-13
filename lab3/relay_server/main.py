from fastapi import FastAPI, Response
from pydantic import BaseModel


app = FastAPI()
app.potentiometer = 0
app.light = 0
app.temp = 0


class Sensor(BaseModel):
    value: int


@app.get('/potentiometer')
def get_potentiometer():
    return Sensor(value=app.potentiometer)


@app.get('/light')
def get_light():
    return Sensor(value=app.light)


@app.get('/temp')
def get_temp():
    return Sensor(value=app.temp)


@app.post("/potentiometer")
def set_potentiometer(response: Response, request: Sensor):
    app.potentiometer = request.value
    response.status_code = 200


@app.post("/light")
def set_light(response: Response, request: Sensor):
    app.light = request.value
    response.status_code = 200


@app.post("/temp")
def set_temp(response: Response, request: Sensor):
    app.temp = request.value
    response.status_code = 200
