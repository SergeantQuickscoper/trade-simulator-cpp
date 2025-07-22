# QuantSim
A trade simulator that calculates market impact and transaction costs with websockets and multithreading.

## Dependencies (install on your system)

- Boost (system, thread)
- CMake 
- Make
- C++17 compatible compiler 
- (Optional, for Windows) MSYS2/MinGW for Unix build tools

## Environment variables setup

**Create a `.env` file in the project root** with the following 6 variables for your websocket server and exchange details:

### Network configuration
```
HOST=ws://example_endpoint
PORT=8080
PATH=/api/data
```
### Simulator configuration
```
EXCHANGE=OKX # currently only OKX is supported (for fee calculations), if you want your own exchange you can append logic in the FeeModel.cpp file map structure
SYMBOL=BTC-USDT-SWAP
INITIAL_CAPITAL=100000.0
```

## WebSocket JSON Message Format

The WebSocket server you're connecting to should send messages in the following JSON format:

```
{
  "timestamp": "2024-01-01T12:00:00Z", // ISO8601 string
  "asks": [ ["price1", "quantity1"], ["price2", "quantity2"], ... ],
  "bids": [ ["price1", "quantity1"], ["price2", "quantity2"], ... ]
}
```
- `asks` and `bids` are arrays of arrays, each containing price and quantity as strings.
- `timestamp` should be in ISO8601 format.

## Mathematical Models Used

### Market Impact Model (Almgren-Chriss Model)
The market impact of a trade is taken as the sum of temporary and permanent impact:

```
Temporary Impact = temporaryImpactFactor × sqrt(|orderSize| / dailyVolume) × currentPrice
Permanent Impact = permanentImpactFactor × (|orderSize| / dailyVolume) × currentPrice
Market Impact = Temporary Impact + Permanent Impact
```
- **temporaryImpactFactor** and **permanentImpactFactor** are configurable parameters.
- **orderSize** is the size of the trade.
- **dailyVolume** is the estimated daily traded volume for the asset.
- **currentPrice** is the current mid price of the asset.

### Slippage Model (Quantile Regression)
Slippage is estimated using historical price and volume data, with quantile regression to estimate volatility:

```
slippage = currentPrice × volatility × sqrt(|orderSize| / avgVolume)
```
- **volatility** is estimated as a quantile (e.g., 95th percentile) of historical returns (historical being from streamed data onwards).
- **avgVolume** is the average historical trade volume.
- **orderSize** is the size of the trade.
- **currentPrice** is the current mid price.

### Fee Model
Fees are calculated based on the exchange, fee tier, and whether the order is a maker or taker:

```
fee = orderSize × price × feeRate
```
- **feeRate** is determined by the exchange and fee tier (only OKX maker/taker fees for now).
- **orderSize** is the size of the trade.
- **price** is the execution price.



## Running it yourself

To compile, execute the following commands in the root folder (For Windows use MINGW/MSYS2), ensure cmake and make are installed:
```
./build.sh
cd build
```
And run the executable.

Authored by: Don Chacko <donisepic30@gmail.com>
