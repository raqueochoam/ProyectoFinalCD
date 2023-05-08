const express = require('express')
const app = express()
const path = require('path')

//settings
app.set('port', 3000)

//midlewares
app.use(express.static(path.join(__dirname, 'public')))

//routes
app.get('/', (req, res)=>{
    res.send('Bienvenido')
})


app.listen(app.get('port'), ()=> {
    console.log(`aplicacion corriendo en el puerto ${app.get('port')}`)
})