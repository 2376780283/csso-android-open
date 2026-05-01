//========= SVG Edge Effects Library - OUTWARD GLOW VERSION =============//
//
// Библиотека эффектов для SVG изображений с размытием НАРУЖУ от краёв
//
//=============================================================================//

#ifndef SVGEDGEEFFECTS_H
#define SVGEDGEEFFECTS_H
#pragma once

#include <cstdint>
#include <cstring>
#include <algorithm>

namespace SVGEffects
{
	//-----------------------------------------------------------------------------
	// Вспомогательная функция: получить альфа-канал пикселя
	//-----------------------------------------------------------------------------
	inline uint8_t GetAlpha(const uint8_t* data, int x, int y, int width, int height)
	{
		if (x < 0 || x >= width || y < 0 || y >= height)
			return 0;
		return data[(y * width + x) * 4 + 3]; // RGBA формат, альфа в позиции 3
	}

	//-----------------------------------------------------------------------------
	// НОВЫЙ ЭФФЕКТ: Внешнее свечение (размытие наружу от краёв)
	// 
	// Создаёт ореол/свечение вокруг иконки заданного цвета
	// Размытие распространяется НАРУЖУ от краёв, не затрагивая саму иконку
	//-----------------------------------------------------------------------------
	inline void OutwardGlow(uint8_t* data, int width, int height, 
	                        uint8_t glowR, uint8_t glowG, uint8_t glowB,
	                        int glowRadius = 8, int glowIntensity = 180)
	{
		if (!data || width <= 0 || height <= 0 || glowRadius <= 0)
			return;

		// Создаём временный буфер для хранения карты расстояний до краёв
		uint8_t* distanceMap = new uint8_t[width * height];
		memset(distanceMap, 0, width * height);

		// Шаг 1: Создаём карту расстояний от каждого прозрачного пикселя до ближайшего непрозрачного
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				int idx = y * width + x;
				uint8_t alpha = data[idx * 4 + 3];

				// Если пиксель уже непрозрачный - пропускаем
				if (alpha > 200) // Считаем непрозрачным если альфа > 200
				{
					distanceMap[idx] = 0;
					continue;
				}

				// Ищем ближайший непрозрачный пиксель в радиусе glowRadius
				int minDistance = glowRadius + 1;
				
				for (int dy = -glowRadius; dy <= glowRadius; dy++)
				{
					for (int dx = -glowRadius; dx <= glowRadius; dx++)
					{
						int nx = x + dx;
						int ny = y + dy;

						if (nx < 0 || nx >= width || ny < 0 || ny >= height)
							continue;

						uint8_t neighborAlpha = data[(ny * width + nx) * 4 + 3];
						
						// Если сосед непрозрачный
						if (neighborAlpha > 200)
						{
							int distance = abs(dx) + abs(dy); // Manhattan distance (быстрее чем Euclidean)
							if (distance < minDistance)
								minDistance = distance;
						}
					}
				}

				// Сохраняем расстояние (0 = далеко, 255 = близко к краю)
				if (minDistance <= glowRadius)
				{
					// Инвертируем: чем ближе к краю, тем ярче свечение
					distanceMap[idx] = (uint8_t)(255 * (glowRadius - minDistance) / glowRadius);
				}
			}
		}

		// Шаг 2: Применяем размытие к карте расстояний (для плавности свечения)
		uint8_t* blurredMap = new uint8_t[width * height];
		memcpy(blurredMap, distanceMap, width * height);

		// Box blur для сглаживания (3 прохода)
		for (int pass = 0; pass < 3; pass++)
		{
			for (int y = 1; y < height - 1; y++)
			{
				for (int x = 1; x < width - 1; x++)
				{
					int idx = y * width + x;
					int sum = 0;
					int count = 0;

					for (int dy = -1; dy <= 1; dy++)
					{
						for (int dx = -1; dx <= 1; dx++)
						{
							int nIdx = (y + dy) * width + (x + dx);
							sum += distanceMap[nIdx];
							count++;
						}
					}

					blurredMap[idx] = (uint8_t)(sum / count);
				}
			}
			memcpy(distanceMap, blurredMap, width * height);
		}

		// Шаг 3: Применяем свечение к пикселям
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				int idx = y * width + x;
				int pixelIdx = idx * 4;

				uint8_t originalAlpha = data[pixelIdx + 3];
				uint8_t glowStrength = blurredMap[idx];

				// Только для прозрачных/полупрозрачных пикселей применяем свечение
				if (originalAlpha < 200 && glowStrength > 0)
				{
					// Вычисляем итоговую альфу свечения
					uint8_t glowAlpha = (glowStrength * glowIntensity) / 255;

					// Если есть исходный цвет - смешиваем, иначе используем цвет свечения
					if (originalAlpha > 0)
					{
						// Смешиваем с оригинальным цветом
						float t = (float)glowAlpha / 255.0f;
						data[pixelIdx + 0] = (uint8_t)(data[pixelIdx + 0] * (1 - t) + glowR * t);
						data[pixelIdx + 1] = (uint8_t)(data[pixelIdx + 1] * (1 - t) + glowG * t);
						data[pixelIdx + 2] = (uint8_t)(data[pixelIdx + 2] * (1 - t) + glowB * t);
						data[pixelIdx + 3] = std::max(originalAlpha, glowAlpha);
					}
					else
					{
						// Полностью прозрачный пиксель - используем цвет свечения
						data[pixelIdx + 0] = glowR;
						data[pixelIdx + 1] = glowG;
						data[pixelIdx + 2] = glowB;
						data[pixelIdx + 3] = glowAlpha;
					}
				}
			}
		}

		delete[] distanceMap;
		delete[] blurredMap;
	}

	//-----------------------------------------------------------------------------
	// УЛУЧШЕННАЯ ВЕРСИЯ: Более быстрое внешнее свечение с радиальным градиентом
	//-----------------------------------------------------------------------------
	inline void FastOutwardGlow(uint8_t* data, int width, int height,
	                             uint8_t glowR, uint8_t glowG, uint8_t glowB,
	                             int glowRadius = 8)
	{
		if (!data || width <= 0 || height <= 0 || glowRadius <= 0)
			return;

		// Создаём маску краёв (где находятся границы непрозрачной части)
		bool* edgeMask = new bool[width * height];
		memset(edgeMask, 0, width * height);

		// Находим края (пиксели на границе непрозрачной области)
		for (int y = 1; y < height - 1; y++)
		{
			for (int x = 1; x < width - 1; x++)
			{
				int idx = y * width + x;
				uint8_t alpha = data[idx * 4 + 3];

				// Если пиксель непрозрачный
				if (alpha > 200)
				{
					// Проверяем соседей - если хоть один прозрачный, это край
					bool isEdge = false;
					for (int dy = -1; dy <= 1 && !isEdge; dy++)
					{
						for (int dx = -1; dx <= 1 && !isEdge; dx++)
						{
							if (dx == 0 && dy == 0) continue;
							
							int nIdx = (y + dy) * width + (x + dx);
							if (data[nIdx * 4 + 3] < 200)
							{
								isEdge = true;
							}
						}
					}
					edgeMask[idx] = isEdge;
				}
			}
		}

		// Применяем свечение от краёв наружу
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				int idx = y * width + x;
				int pixelIdx = idx * 4;
				uint8_t alpha = data[pixelIdx + 3];

				// Только для прозрачных/полупрозрачных пикселей
				if (alpha < 200)
				{
					// Находим ближайший край
					int minDist = glowRadius + 1;

					for (int dy = -glowRadius; dy <= glowRadius; dy++)
					{
						for (int dx = -glowRadius; dx <= glowRadius; dx++)
						{
							int nx = x + dx;
							int ny = y + dy;

							if (nx < 0 || nx >= width || ny < 0 || ny >= height)
								continue;

							int nIdx = ny * width + nx;
							if (edgeMask[nIdx])
							{
								int dist = abs(dx) + abs(dy);
								if (dist < minDist)
									minDist = dist;
							}
						}
					}

					// Если нашли край в радиусе свечения
					if (minDist <= glowRadius)
					{
						// Вычисляем силу свечения (затухает с расстоянием)
						float strength = 1.0f - (float)minDist / (float)glowRadius;
						uint8_t glowAlpha = (uint8_t)(strength * 220); // Максимум 220

						// Смешиваем с существующим цветом
						if (alpha > 0)
						{
							float t = strength * 0.7f; // Коэффициент смешивания
							data[pixelIdx + 0] = (uint8_t)(data[pixelIdx + 0] * (1 - t) + glowR * t);
							data[pixelIdx + 1] = (uint8_t)(data[pixelIdx + 1] * (1 - t) + glowG * t);
							data[pixelIdx + 2] = (uint8_t)(data[pixelIdx + 2] * (1 - t) + glowB * t);
							data[pixelIdx + 3] = std::max(alpha, glowAlpha);
						}
						else
						{
							// Полностью прозрачный пиксель
							data[pixelIdx + 0] = glowR;
							data[pixelIdx + 1] = glowG;
							data[pixelIdx + 2] = glowB;
							data[pixelIdx + 3] = glowAlpha;
						}
					}
				}
			}
		}

		delete[] edgeMask;
	}

	//-----------------------------------------------------------------------------
	// ОПТИМИЗИРОВАННАЯ ВЕРСИЯ: Самая быстрая, использует расширение альфа-канала
	//-----------------------------------------------------------------------------
	inline void QuickOutwardGlow(uint8_t* data, int width, int height,
	                              uint8_t glowR, uint8_t glowG, uint8_t glowB,
	                              int glowRadius = 8)
	{
		if (!data || width <= 0 || height <= 0 || glowRadius <= 0)
			return;

		// Создаём копию альфа-канала
		uint8_t* alphaMap = new uint8_t[width * height];
		for (int i = 0; i < width * height; i++)
			alphaMap[i] = data[i * 4 + 3];

		// Расширяем альфа-канал наружу (dilation)
		for (int pass = 0; pass < glowRadius; pass++)
		{
			uint8_t* tempMap = new uint8_t[width * height];
			memcpy(tempMap, alphaMap, width * height);

			for (int y = 1; y < height - 1; y++)
			{
				for (int x = 1; x < width - 1; x++)
				{
					int idx = y * width + x;
					
					// Если пиксель прозрачный, проверяем соседей
					if (alphaMap[idx] < 50)
					{
						uint8_t maxNeighbor = 0;
						
						for (int dy = -1; dy <= 1; dy++)
						{
							for (int dx = -1; dx <= 1; dx++)
							{
								if (dx == 0 && dy == 0) continue;
								
								int nIdx = (y + dy) * width + (x + dx);
								maxNeighbor = std::max(maxNeighbor, alphaMap[nIdx]);
							}
						}

						// Затухание свечения
						if (maxNeighbor > 50)
							tempMap[idx] = (uint8_t)(maxNeighbor * 0.7f);
					}
				}
			}

			memcpy(alphaMap, tempMap, width * height);
			delete[] tempMap;
		}

		// Применяем расширенную альфу и цвет свечения
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				int idx = y * width + x;
				int pixelIdx = idx * 4;

				uint8_t originalAlpha = data[pixelIdx + 3];
				uint8_t expandedAlpha = alphaMap[idx];

				// Применяем свечение только к прозрачным областям
				if (originalAlpha < 200 && expandedAlpha > originalAlpha)
				{
					uint8_t glowAlpha = expandedAlpha - originalAlpha;

					if (originalAlpha > 0)
					{
						// Смешиваем цвета
						float t = (float)glowAlpha / 255.0f;
						data[pixelIdx + 0] = (uint8_t)(data[pixelIdx + 0] * (1 - t) + glowR * t);
						data[pixelIdx + 1] = (uint8_t)(data[pixelIdx + 1] * (1 - t) + glowG * t);
						data[pixelIdx + 2] = (uint8_t)(data[pixelIdx + 2] * (1 - t) + glowB * t);
					}
					else
					{
						// Чистое свечение
						data[pixelIdx + 0] = glowR;
						data[pixelIdx + 1] = glowG;
						data[pixelIdx + 2] = glowB;
					}
					
					data[pixelIdx + 3] = expandedAlpha;
				}
			}
		}

		delete[] alphaMap;
	}

} // namespace SVGEffects

#endif // SVGEDGEEFFECTS_H
