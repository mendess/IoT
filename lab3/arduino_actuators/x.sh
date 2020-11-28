
case "$1" in
    t | T)
        route=temp
        ;;
    l | L)
        route=light
        ;;
    p | P)
        route=potentiometer
        ;;
    *)
        echo "invalid route"
        exit 1
esac

curl \
    --header "Content-Type: application/json" \
    --request POST \
    --data "{\"value\": $2}" \
    https://iot-lab3.herokuapp.com/$route
echo
